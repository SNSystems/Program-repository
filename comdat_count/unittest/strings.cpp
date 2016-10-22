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

#include "strings.h"

// Standard library includes
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iterator>
#include <stdexcept>

// scanlib includes
#include "elf_helpers.hpp"

// (ctor)
// ~~~~~~
strings::strings ()
        : data_{'\0'} { // Insert the initial null character.
}

// readonly
// ~~~~~~~~
void strings::readonly () {
    writable_ = false;
}

// append
// ~~~~~~
std::size_t strings::append (std::string const & str) {
    return this->append (str.data (), str.length ());
}
std::size_t strings::append (char const * str) {
    return this->append (str, std::strlen (str));
}
std::size_t strings::append (char const * str, std::size_t length) {
    assert (writable_);
    if (!writable_) {
        throw std::runtime_error ("cannot write to a read-only string table!");
    }
    std::size_t const result = data_.size ();
    data_.reserve (result + length + 1);
    std::copy (str, str + length, std::back_inserter (data_));
    data_.push_back ('\0');
    return result;
}


// *********************
// create_string_section
// ~~~~~~~~~~~~~~~~~~~~~
Elf_Scn * create_string_section (Elf * const elf, strings const & content, std::string const & name,
                                 strings * const section_names) {

    assert (elf != nullptr);
    assert (section_names != nullptr);

    Elf_Scn * const string_section = elf::newscn (elf);
    assert (string_section != nullptr);
    std::size_t const name_offset = section_names->append (name);
    {
        Elf_Data * const data = elf::newdata (string_section);
        assert (data != nullptr);
        data->d_align = alignof (char);
        data->d_buf = const_cast<char *> (content.data ());
        data->d_off = 0;
        data->d_type = ELF_T_BYTE;
        data->d_size = content.size ();
        data->d_version = EV_CURRENT;
    }
    {
        GElf_Shdr shdr = gelf::getshdr (string_section);
        shdr.sh_name = name_offset;
        shdr.sh_type = SHT_STRTAB;
        shdr.sh_flags = SHF_STRINGS;
        gelf::update_shdr (string_section, shdr);
    }
    return string_section;
}

// eof strings.cpp
