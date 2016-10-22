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

#include "elf_scanner.hpp"

#include <array>
#include <cassert>
#include <iterator>

#include "elf_helpers.hpp"

// (ctor)
// ~~~~~~
elf_scanner::elf_scanner (Elf * const elf)
        : elf_ (elf)
        , is_le_ (elf_is_le (elf)) {}

// scan
// ~~~~
void elf_scanner::scan (callback const & cb) {
    Elf_Scn * section = nullptr;
    while ((section = elf_nextscn (elf_, section)) != nullptr) {
        GElf_Shdr shdr = gelf::getshdr (section);
        if (shdr.sh_type == SHT_GROUP) {
            this->scan_group_section (section, shdr, cb);
        }
    }
}

// get_le
// ~~~~~~
std::uint32_t elf_scanner::get_le (std::uint8_t const * v) {
    return (static_cast<std::uint32_t> (v[3]) << 24) | (static_cast<std::uint32_t> (v[2]) << 16) |
           (static_cast<std::uint32_t> (v[1]) << 8) | v[0];
}

// get_be
// ~~~~~~
std::uint32_t elf_scanner::get_be (std::uint8_t const * v) {
    return (static_cast<std::uint32_t> (v[0]) << 24) | (static_cast<std::uint32_t> (v[1]) << 16) |
           (static_cast<std::uint32_t> (v[2]) << 8) | v[3];
}

// elf_is_le
// ~~~~~~~~~
bool elf_scanner::elf_is_le (Elf * const elf) {
    GElf_Ehdr ehdr = gelf::getehdr (elf);
    return ehdr.e_ident[EI_DATA] == ELFDATA2LSB;
}

void elf_scanner::record_member (state * const st, std::uint32_t v) {
    // The first 4 byte value in a group section is the flag word:
    // if it's not GRP_COMDAT then skip its contents.
    if (st->member_count++ == 0) {
        if (v != GRP_COMDAT) {
            st->n = st->shdr.sh_size;
            assert (st->total_size == 0);
        }
    } else {
        Elf_Scn * const escn = elf::getscn (elf_, v);
        st->total_size += gelf::getshdr (escn).sh_size;
    }
}

// scan_group_section
// ~~~~~~~~~~~~~~~~~~
void elf_scanner::scan_group_section (Elf_Scn * section, GElf_Shdr const & shdr,
                                      callback const & cb) {
    assert (shdr.sh_type == SHT_GROUP);

    std::array<std::uint8_t, 4> bytes;
    auto v_it = std::begin (bytes);
    auto v_end = std::end (bytes);

    state st (shdr);

    // TODO: calling elf_getdata() and assuming that the library doesn't know about
    // group sections and this code is responsible for the byte-sex conversion. If we're
    // linked against a version that does know about it, we'll surely break. Use
    // elf_rawdata() instead?
    // TODO: add support for SHT_GROUP to libelf?!
    Elf_Data * data = nullptr;
    decltype (shdr.sh_size) n = 0;

    if (shdr.sh_size % sizeof (std::uint32_t) != 0) {
        throw std::runtime_error ("SHT_GROUP sections must be a multiple of 4 bytes");
    }

    // Some versions of libelf understand that an SHT_GROUP section contains an array of
    // ELF_T_WORD, and some do not. To minimize the dependency on a particular version or
    // implementation, I'm requesting the raw section data (i.e. not translated by the
    // library) and doing the byte swapping myself.

    while (n < shdr.sh_size && (data = elf_rawdata (section, data)) != nullptr) {

        for (auto p = static_cast<std::uint8_t const *> (data->d_buf), end = p + data->d_size;
             p < end; ++p, ++n) {

            assert (n < shdr.sh_size);
            *(v_it++) = *p;
            if (v_it == v_end) {
                // We've read 4 bytes. Turn that into a section index.
                std::uint32_t const index =
                    is_le_ ? get_le (bytes.data ()) : get_be (bytes.data ());
                this->record_member (&st, index);

                // Start the next set of 4 bytes.
                v_it = std::begin (bytes);
            }
        }
    }

    // Check that we consumed a multiple of 4 bytes of data.
    // TODO: issue a warning that the group contents weren't valid?
    // assert (v_it == std::begin (bytes));

    if (st.total_size > 0) {
        cb (this->group_identifier (shdr), st.total_size);
    }
}

// group_identifier
// ~~~~~~~~~~~~~~~~
std::string elf_scanner::group_identifier (GElf_Shdr const & group) {
    Elf_Scn * const identifying_symbol_table = elf_getscn (elf_, group.sh_link);
    GElf_Sym const identifying_symbol = gelf::getsym (identifying_symbol_table, group.sh_info);
    auto const symbol_names_section_index = gelf::getshdr (identifying_symbol_table).sh_link;
    return elf::strptr (elf_, symbol_names_section_index, identifying_symbol.st_name);
}
// eof elf_scanner.cpp
