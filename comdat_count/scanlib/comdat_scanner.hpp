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

#ifndef COMDAT_SCANNER_H
#define COMDAT_SCANNER_H

#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/filesystem/path.hpp>

#include "digests.hpp"
#include "flags.hpp"

struct Elf;

class comdat_scanner_base {
public:
    virtual ~comdat_scanner_base ();
    virtual void scan (boost::filesystem::path const & user_file_path, struct Elf * const elf) = 0;
    virtual void skip (boost::filesystem::path const & user_file_path, struct Elf * const elf) = 0;
};

class comdat_scanner final : public comdat_scanner_base {
public:
    explicit comdat_scanner (output_flags const & ofl);

    void scan (boost::filesystem::path const & user_file_path, struct Elf * const elf) override;
    void skip (boost::filesystem::path const & user_file_path, struct Elf * const elf) override;

    std::ostream & dump (std::ostream & os) const;



    struct output {
        std::uint64_t largest;
        unsigned instances;
        std::uint64_t wasted;
    };
    using output_vector = std::vector<output>;
    static output_vector filter (output_vector & src);


    struct value {
        /// The total size of all instances encountered.
        std::uint64_t total_size;
        /// The size of the largest instances encountered.
        std::uint64_t largest;
        /// The number of instances encountered.
        unsigned instances;
    };
    typedef std::unordered_map<std::string, value> comdat_map;

    static output_vector build_output_vector (comdat_map const & cm);


    struct sizes {
        std::uint64_t actual;
        std::size_t waste;
    };
    static sizes total_comdat_size (comdat_map const & cm);

private:
    // Returns a user string for the given path/elf combination.
    static std::string get_name (boost::filesystem::path const & path, Elf * const elf);

    output_flags const ofl_;
    mutable digests digests_;
    mutable std::mutex comdat_lock_;
    comdat_map comdat_count_;
};

bool operator== (comdat_scanner::output const & lhs, comdat_scanner::output const & rhs);
bool operator!= (comdat_scanner::output const & lhs, comdat_scanner::output const & rhs);
bool operator< (comdat_scanner::output const & lhs, comdat_scanner::output const & rhs);
std::ostream & operator<< (std::ostream & os, comdat_scanner::output const & d);


bool operator== (comdat_scanner::sizes const & lhs, comdat_scanner::sizes const & rhs);
bool operator!= (comdat_scanner::sizes const & lhs, comdat_scanner::sizes const & rhs);
std::ostream & operator<< (std::ostream & os, comdat_scanner::sizes const & d);

std::ostream & operator<< (std::ostream & os, comdat_scanner const & d);

#endif // COMDAT_SCANNER_H
// eof comdat_scanner.h
