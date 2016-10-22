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

#ifndef UNITTEST_STRINGS_H
#define UNITTEST_STRINGS_H

#include <cstddef>
#include <string>
#include <vector>

class strings {
public:
    strings ();

    std::size_t append (std::string const & str);
    std::size_t append (char const * str);
    std::size_t append (char const * str, std::size_t length);

    void readonly ();
    char const * data () const {
        return data_.data ();
    }
    std::size_t size () const {
        return data_.size ();
    }

private:
    std::vector<char> data_;
    bool writable_ = true;
};

struct Elf;
struct Elf_Scn;

Elf_Scn * create_string_section (Elf * const elf, strings const & content, std::string const & name,
                                 strings * const section_names);

#endif // UNITTEST_STRINGS_H
// eof strings.h
