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

#ifndef UNITTEST_SYMBOL_SECTION_H
#define UNITTEST_SYMBOL_SECTION_H

#include <cstdlib>
#include <string>
#include <vector>

#include <gelf.h>
#include "strings.h"

class symbol_section {
public:
    explicit symbol_section (Elf * elf);
    ~symbol_section ();

    Elf_Scn * section () {
        return section_;
    }

    /// Add a symbol with the given name and refering to an offset within an existing section.
    /// \returns The index of the newly added symbol.
    std::size_t add (std::string const & name, Elf_Scn * section, std::size_t offset,
                     std::size_t size);

    /// Writes the symbol table.
    void commit (strings * const section_names);

private:
    bool is_64_bit ();
    template <typename SymbolContainer>
    Elf_Data * create_data (Elf_Scn * const symbol_section,
                            SymbolContainer * const container) const;

    Elf * elf_;
    Elf_Scn * section_;
    std::vector<GElf_Sym> symbols_;

    /// Storage for 32-bit symbols if we're writing a 32-bit ELF.
    std::vector<Elf32_Sym> sym32_;
    /// Storage for 64-bit symbols if we're writing a 64-bit ELF.
    std::vector<Elf64_Sym> sym64_;

    /// A container that will hold the symbol names.
    strings names_;

    /// True until commit() is called. Once commit() has been called, use of either add() or
    /// commit() will assert/raise an exception.
    bool writable_;
};

#endif // UNITTEST_SYMBOL_SECTION_H
// eof symbol_section.h
