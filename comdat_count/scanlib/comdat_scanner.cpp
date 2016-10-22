// Copyright (c) 2016 by SN Systems Ltd., Sony Interactive Entertainment Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "comdat_scanner.hpp"

// Standard Library
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <future>
#include <numeric>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

// Local includes
#include "consumer.hpp"
#include "elf_helpers.hpp"
#include "elf_scanner.hpp"
#include "print.hpp"

// -------------------------------
// comdat scanner base
// -------------------------------
// (dtor)
// ~~~~~~
comdat_scanner_base::~comdat_scanner_base () {}



// -------------------------------
// comdat scanner :: output
// -------------------------------

bool operator== (comdat_scanner::output const & lhs, comdat_scanner::output const & rhs) {
    return std::tie (lhs.largest, lhs.instances, lhs.wasted) ==
           std::tie (rhs.largest, rhs.instances, rhs.wasted);
}

bool operator!= (comdat_scanner::output const & lhs, comdat_scanner::output const & rhs) {
    return !operator== (lhs, rhs);
}

bool operator< (comdat_scanner::output const & lhs, comdat_scanner::output const & rhs) {
    return std::tie (lhs.largest, lhs.instances, lhs.wasted) <
           std::tie (rhs.largest, rhs.instances, rhs.wasted);
}

std::ostream & operator<< (std::ostream & os, comdat_scanner::output const & d) {
    return os << "{ largest:" << d.largest << ", instances:" << d.instances
              << ", wasted:" << d.wasted << " }";
}


// -------------------------------
// comdat scanner :: sizes
// -------------------------------

bool operator== (comdat_scanner::sizes const & lhs, comdat_scanner::sizes const & rhs) {
    return std::tie (lhs.actual, lhs.waste) == std::tie (rhs.actual, rhs.waste);
}

bool operator!= (comdat_scanner::sizes const & lhs, comdat_scanner::sizes const & rhs) {
    return !operator== (lhs, rhs);
}

std::ostream & operator<< (std::ostream & os, comdat_scanner::sizes const & d) {
    return os << "{ actual: " << d.actual << ", waste:" << d.waste << " }";
}


// -------------------------------
// comdat scanner
// -------------------------------

// (ctor)
// ~~~~~~
comdat_scanner::comdat_scanner (output_flags const & ofl)
        : ofl_ (ofl)
        , digests_ ()
        , comdat_lock_ ()
        , comdat_count_ () {}

// scan
// ~~~~
void comdat_scanner::scan (boost::filesystem::path const & user_file_path, Elf * const elf) {
    assert (elf != nullptr);
    if (ofl_.verbose) {
        print_cout ("Scan: ", this->get_name (user_file_path, elf));
    }

    digests_.add_elf (elf);

    elf_scanner esc (elf);
    esc.scan ([this](std::string const & identifier, std::uint64_t size) {
        std::lock_guard<std::mutex> guard (comdat_lock_);
        value & val = comdat_count_[identifier];
        val.total_size += size;
        val.largest = std::max (val.largest, size);
        ++val.instances;
    });
}

// skip
// ~~~~
void comdat_scanner::skip (boost::filesystem::path const & user_file_path, Elf * const elf) {
    ::elf_getarhdr (elf);
    if (::elf_errno () != 0 || elf == nullptr) {
        if (!ofl_.quiet) {
            print_cout ("Skipping: ", this->get_name (user_file_path, elf));
        }
    }
}

// get_name [static]
// ~~~~~~~~
std::string comdat_scanner::get_name (boost::filesystem::path const & path, Elf * const elf) {
    Elf_Arhdr const * const arh = ::elf_getarhdr (elf);
    if (arh == nullptr || ::elf_errno () != 0) {
        return path.string ();
    } else {
        std::ostringstream str;
        str << path << " (" << arh->ar_name << ")";
        return str.str ();
    }
}

// build_output_vector [static]
// ~~~~~~~~~~~~~~~~~~~
auto comdat_scanner::build_output_vector (comdat_map const & cm) -> output_vector {
    output_vector counts;
    counts.reserve (cm.size ());
    for (auto const & v : cm) {
        auto const instances = v.second.instances;
        if (instances > 1) {
            auto const largest = v.second.largest;
            auto const wasted = v.second.total_size - v.second.largest;
            counts.push_back ({largest, instances, wasted});
        }
    }
    std::sort (std::begin (counts), std::end (counts));
    return counts;
}

// filter [static]
// ~~~~~~
/// Scans through the "src" container removing all of the points that are
/// "close" to one another. The resulting collection of points is returned in
/// "dest".
auto comdat_scanner::filter (output_vector & src) -> output_vector {
    output_vector dest;

    static double const min_distance = 0.05;
    struct point {
        double x;
        double y;
    };

    auto first = std::begin (src);
    auto last = std::end (src);
    // The outer loop iterates over the source container examining each point in turn.
    // Note that there are two points where 'first' may be incremented.
    while (first != last) {
        output v = *(first++);
        point const vpos{std::log10 (v.largest), std::log10 (v.instances)};

        // The inner loop finds points close to vpos.
        for (auto it = first; it != last; ++it) {
            point const it_pos{std::log10 (it->largest), std::log10 (it->instances)};

            // Compute the distance between vpos and it_pos on the graph. Good old Pythagoras.
            point const dim = point{std::max (vpos.x, it_pos.x) - std::min (vpos.x, it_pos.x),
                                    std::max (vpos.y, it_pos.y) - std::min (vpos.y, it_pos.y)};
            auto const distance = std::sqrt (dim.x * dim.x + dim.y * dim.y);
            if (distance < min_distance) {
                v.wasted = std::max (v.wasted, it->wasted);

                // Swap this point out so that we don't consider it the next time
                // round the outer loop.
                std::swap (*(first++), *it);
            }
        }

        dest.push_back (v);
    }

    return dest;
}

// total_comdat_size [static]
// ~~~~~~~~~~~~~~~~~
/// Returns the actual and wasted COMDAT totals.
auto comdat_scanner::total_comdat_size (comdat_map const & cm) -> sizes {
    auto acc_fn = [](sizes const & s, comdat_map::value_type const & value) -> sizes {
        return {
            s.actual + value.second.total_size,
            s.waste + value.second.total_size - value.second.largest,
        };
    };
    return std::accumulate (std::begin (cm), std::end (cm), sizes{0, 0}, acc_fn);
}

// dump
// ~~~~
std::ostream & comdat_scanner::dump (std::ostream & os) const {

    // When this function is called, we shouldn't still be building the COMDAT records.
    // Nevertheless, I take the lock just in case.
    std::lock_guard<std::mutex> comdat_lock (comdat_lock_);

    std::future<output_vector> counts_future =
        std::async (build_output_vector, std::cref (comdat_count_));
    std::future<sizes> total_size_future =
        std::async (total_comdat_size, std::cref (comdat_count_));

    // This step removes all of the duplicate graph points, returning a collection with only the
    // largest 'wasted'
    // value for each deleted point. The argument collection is reordered.
    std::future<md5::digest> digest_future = std::async ([this]() { return digests_.final (); });

    auto counts = counts_future.get ();
    std::future<output_vector> counts2_future = std::async (filter, std::ref (counts));

    os << "# MD5: " << md5::context::digest_hex (digest_future.get ()) << '\n';
    os << "# Filtered " << comdat_count_.size () - counts.size () << " COMDATs with 1 instance\n";

    auto const & counts2 = counts2_future.get ();
    os << "# Then trimmed " << counts.size () - counts2.size () << " similar points\n";
    os << "# Result has " << counts2.size () << " points\n";

    auto const & total_size = total_size_future.get ();
    os << "#> Total:" << total_size.actual << '\n' << "#> Wasted:" << total_size.waste << '\n';

    os << "Size Instances Total\n";
    for (auto const & v : counts2) {
        assert (v.instances > 1);
        os << v.largest << ' ' << v.instances << ' ' << v.wasted << '\n';
    }
    return os;
}


std::ostream & operator<< (std::ostream & os, comdat_scanner const & d) {
    return d.dump (os);
}

// eof comdat_scanner.cpp
