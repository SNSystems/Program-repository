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

#ifndef UNITTEST_SECTIONS_H
#define UNITTEST_SECTIONS_H

// Standard library includes
#include <cassert>
#include <cstdint>
#include <type_traits>

// 3rd party includes
#include <gelf.h>

// scanlib includes
#include "elf_helpers.hpp"

// Local includes
#include "symbol_section.h"

template <typename Ty>
Elf_Type as_elf_type ();

template <>
inline Elf_Type as_elf_type<std::uint8_t> () {
    return ELF_T_BYTE;
}
template <>
inline Elf_Type as_elf_type<std::uint32_t> () {
    return ELF_T_WORD;
}

template <typename Container>
Elf_Data * add_section_data (Elf_Scn * const section, Container const * const content) {
    assert (section != nullptr);
    assert (content != nullptr);

    typedef typename Container::value_type value_type;
    Elf_Data * const data = elf::newdata (section);
    data->d_align = alignof (value_type);
    data->d_off = 0;
    data->d_buf = const_cast<value_type *> (content->data ());
    data->d_type = as_elf_type<value_type> ();
    data->d_size = sizeof (value_type) * content->size ();
    data->d_version = EV_CURRENT;
    return data;
}


template <typename Container>
Elf_Scn * create_progbits_alloc_section (Elf * const elf, Container const * const content,
                                         std::size_t const section_name_offset) {
    assert (elf != nullptr);
    assert (content != nullptr);

    Elf_Scn * const section = elf::newscn (elf);
    add_section_data (section, content);

    // Update the fields of the section header.
    GElf_Shdr shdr = gelf::getshdr (section);
    shdr.sh_name = section_name_offset;
    shdr.sh_type = SHT_PROGBITS;
    shdr.sh_flags = SHF_ALLOC;
    gelf::update_shdr (section, shdr);

    return section;
}

// A section of type SHT_GROUP defines a grouping of sections. The name of a symbol from one
// of the containing object's symbol tables provides a signature for the section group. The
// section
// header of the SHT_GROUP section specifies the identifying symbol entry. The sh_link member
// contains the section header index of the symbol table section that contains the entry. The
// sh_info member contains the symbol table index of the identifying entry. The sh_flags member
// of the section header contains the value zero. The name of the section (sh_name) is not
// specified.
template <typename Container>
Elf_Scn * create_group_section (Elf * const elf, std::string const & identifier,
                                symbol_section * const symbols, std::size_t section_name_offset,
                                Container const * const group_data) {
    static_assert (std::is_same<typename std::remove_cv<typename Container::value_type>::type,
                                std::uint32_t>::value,
                   "A group section must contain a vector of uint32_t");
    assert (elf != nullptr);
    assert (symbols != nullptr);
    assert (group_data != nullptr);

    // Create the group section.
    Elf_Scn * const section = elf::newscn (elf);

    // Populate it with data.
    // It's an array of 32-bit values; libelf will deal with the byte swapping on output.
    add_section_data (section, group_data);

    // Add the identifying symbol.
    std::size_t const symbol_index = symbols->add (identifier, section, 0 /*offset*/, 0 /*size*/);

    // Properly configure the section header.
    GElf_Shdr shdr = gelf::getshdr (section);
    shdr.sh_name = section_name_offset;
    shdr.sh_type = SHT_GROUP;
    shdr.sh_flags = 0;
    shdr.sh_info = symbol_index; // symbol table index of the identifying entry.
    shdr.sh_entsize = sizeof (std::uint32_t);
    shdr.sh_link = elf_ndxscn (symbols->section ()); // identifying entry's symbol table.
    gelf::update_shdr (section, shdr);

    return section;
}

Elf_Scn * create_section_names_section (Elf * const elf, strings * const section_names);


#endif // UNITTEST_SECTIONS_H
// eof sections.h
