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

#include "counts.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <ostream>
#include <sstream>

// (ctor)
// ~~~~~~
counts::counts (unsigned total)
        : total_{total}
        , types_{0}
        , unique_{0} {}

counts::counts (counts && rhs)
        : signatures_ ()
        , total_ (rhs.total_)
        , types_ (rhs.types_.load ())
        , unique_ (rhs.unique_.load ()) {

    std::lock_guard<std::mutex> lock (rhs.mut_);
    signatures_ = std::move (rhs.signatures_);
    producers_ = std::move (rhs.producers_);
}

// is_unique
// ~~~~~~~~~
bool counts::is_unique (std::uint64_t signature, std::string const & producer) {
    std::lock_guard<std::mutex> lock (mut_);
    producers_.emplace (simplified_producer_name (std::begin (producer), std::end (producer)));
    return signatures_.emplace (signature).second;
}

// simplified_producer_name
// ~~~~~~~~~~~~~~~~~~~~~~~~
std::string counts::simplified_producer_name (std::string::const_iterator first,
                                              std::string::const_iterator last) {
    auto is_ws = [](char c) { return std::isspace (c); };

    std::string result;
    auto start = first;
    auto stop = start;
    char const * sep = "";

    while (stop != last) {
        // A '-' (introducing a switch name) seems like a good place to stop.
        assert (std::distance (start, last) >= 0);
        if (*start == '-') {
            break;
        }

        // Find the next whitespace character.
        stop = std::find_if (start, last, is_ws);
        result += sep;

        // Append the word we found except if it's "version": I shorten that to "v".
        auto word = std::string (start, stop);
        if (word == "version") {
            word = "v";
        }
        result.append (word);

        // Skip any additional whitespace.
        stop = std::find_if_not (stop, last, is_ws);
        start = stop;

        // Separate the words with a single space.
        sep = " ";
    }
    return result;
}

// producer
// ~~~~~~~~
std::string counts::producer () const {
    std::lock_guard<std::mutex> lock (mut_);
    if (producers_.size () == 0) {
        return "Unknown";
    }

    std::ostringstream result;
    char const * sep = "";
    for (auto const & p : producers_) {
        result << sep << p;
        sep = "/";
    }
    return result.str ();
}

// write
// ~~~~~
std::ostream & counts::write (std::ostream & os) const {
    constexpr char const * indent = "    ";
    return os << "{\n"
              << indent << "\"total\": " << total () << ",\n"
              << indent << "\"types\": " << types () << ",\n"
              << indent << "\"unique\": " << unique () << ",\n"
              << indent << "\"producer\": \"" << producer () << "\"\n"
              << "}\n";
}

// eof counts.cpp
