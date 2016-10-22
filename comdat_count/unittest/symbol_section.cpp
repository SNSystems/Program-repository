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

#include "symbol_section.h"

// Standard library includes
#include <cassert>

// scanlib includes
#include "elf_helpers.hpp"

// (ctor)
// ~~~~~~
symbol_section::symbol_section (Elf * const elf)
        : elf_ (elf)
        , section_ (elf::newscn (elf_))
        , symbols_ ()
        , names_ ()
        , writable_ (true) {

    assert (elf_ != nullptr);
    assert (section_ != nullptr);

    // Build the reserved symbol table entry for index 0 (STN_UNDEF).
    GElf_Sym undef_sym;
    undef_sym.st_name = 0;  // No name
    undef_sym.st_value = 0; // Zero value
    undef_sym.st_size = 0;  // No size
    undef_sym.st_info = 0;  // No type, local binding
    undef_sym.st_other = 0;
    undef_sym.st_shndx = SHN_UNDEF; // No section
    symbols_.push_back (undef_sym);
}

// (dtor)
// ~~~~~~
symbol_section::~symbol_section () {}

// add
// ~~~
std::size_t symbol_section::add (std::string const & name, Elf_Scn * const section,
                                 std::size_t offset, std::size_t size) {
    assert (section != nullptr);
    assert (writable_);

    std::size_t result = symbols_.size ();
    GElf_Sym s;
    s.st_name = names_.append (name);
    s.st_info = ELF64_ST_INFO (STB_GLOBAL, STT_OBJECT);
    s.st_other = STV_DEFAULT;
    s.st_shndx = elf_ndxscn (section);
    s.st_value = offset;
    s.st_size = size;
    symbols_.push_back (s);
    return result;
}

// create_data
// ~~~~~~~~~~~
template <typename SymbolContainer>
Elf_Data * symbol_section::create_data (Elf_Scn * const symbol_section,
                                        SymbolContainer * const container) const {
    assert (symbol_section != nullptr);
    assert (container != nullptr);

    typedef typename SymbolContainer::value_type value_type;
    auto const num_symbols = symbols_.size ();
    container->resize (num_symbols);
    Elf_Data * const data = elf::newdata (symbol_section);
    data->d_align = alignof (value_type);
    data->d_off = 0;
    data->d_buf = container->data ();
    data->d_type = ELF_T_SYM;
    data->d_size = sizeof (value_type) * num_symbols;
    data->d_version = EV_CURRENT;
    return data;
}

// is_64_bit
// ~~~~~~~~~
bool symbol_section::is_64_bit () {
    switch (gelf_getclass (elf_)) {
    case ELFCLASS32:
        return false;
    case ELFCLASS64:
        return true;
    default:
        throw std::runtime_error ("gelf_getclass() returned an unknown class");
    }
}

// commit
// ~~~~~~
void symbol_section::commit (strings * const section_names) {
    assert (section_names != nullptr);
    assert (writable_);

    // No matter what happens in this function, you can't add any more symbols.
    writable_ = false;
    if (symbols_.size () > 0) {
        // Create the real ELF Symbol structures (either 32 or 64-bit) and copy each of the
        // instances to it.
        Elf_Data * const symbol_data = this->is_64_bit () ? this->create_data (section_, &sym64_)
                                                          : this->create_data (section_, &sym32_);
        for (std::size_t index = 0, size = symbols_.size (); index < size; ++index) {
            gelf::update_sym (symbol_data, index, symbols_[index]);
        }

        // Now that we've build the final symbol table respresentation in either sym32_ or
        // sym64_, the symbols_ vector is no longer needed.
        symbols_.clear ();

        Elf_Scn * const symbol_string_section =
            create_string_section (elf_,
                                   names_,         // string table contents
                                   ".strtab",      // string table name
                                   section_names); // section header strings.

        GElf_Shdr shdr = gelf::getshdr (section_);
        shdr.sh_name = section_names->append (".symtab");
        shdr.sh_type = SHT_SYMTAB;
        shdr.sh_flags = 0;
        shdr.sh_link = elf_ndxscn (symbol_string_section);
        shdr.sh_info = 0; // FIXME: "One greater than the symbol table index of the last local
                          // symbol, STB_LOCAL"?
        gelf::update_shdr (section_, shdr);
    }
}
// eof symbol_section.cpp
