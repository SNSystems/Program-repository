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

#ifndef COUNTS_HPP
#define COUNTS_HPP

#include <atomic>
#include <iosfwd>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>

class counts {
public:
    explicit counts (unsigned total);
    counts (counts && rhs);

    void add_type (std::uint64_t signature, std::string const & producer) {
        if (is_unique (signature, producer)) {
            ++unique_;
        }
        ++types_;
    }

    std::ostream & write (std::ostream & os) const;

    unsigned total () const {
        return total_;
    }
    unsigned types () const {
        return types_;
    }
    unsigned unique () const {
        return unique_;
    }
    std::string producer () const;

private:
    bool is_unique (std::uint64_t signature, std::string const & producer);

    static std::string simplified_producer_name (std::string::const_iterator first,
                                                 std::string::const_iterator last);

    mutable std::mutex mut_;
    std::unordered_set<std::uint64_t> signatures_;
    std::set<std::string> producers_;

    unsigned total_;
    std::atomic<unsigned> types_;
    std::atomic<unsigned> unique_;
};

inline std::ostream & operator<< (std::ostream & os, counts const & c) {
    return c.write (os);
}

#endif // COUNTS_HPP
// eof counts.hpp
