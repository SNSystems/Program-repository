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

#include "sections.h"

namespace {
    // Newer versions of libelf have elfx_update_shstrndx(), but I can't rely on its presence.
    void update_shstrndx (Elf * const elf, std::size_t strtab) {
        assert (elf != nullptr);

        GElf_Ehdr ehdr = gelf::getehdr (elf);
        if (strtab < SHN_LORESERVE) {
            ehdr.e_shstrndx = strtab;
        } else {
            Elf_Scn * const section = elf::getscn (elf, 0);
            assert (section != nullptr);
            GElf_Shdr shdr = gelf::getshdr (section);
            shdr.sh_link = strtab;
            gelf::update_shdr (section, shdr);
            ehdr.e_shstrndx = SHN_XINDEX;
        }
        gelf::update_ehdr (elf, ehdr);
    }
}

Elf_Scn * create_section_names_section (Elf * const elf, strings * const section_names) {
    assert (elf != nullptr);
    assert (section_names != nullptr);

    Elf_Scn * const section =
        create_string_section (elf, *section_names, ".shstrtab", section_names);
    section_names->readonly ();
    update_shstrndx (elf, elf_ndxscn (section));
    return section;
}

// eof sections.cpp
