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

#ifndef ELF_HELPERS_H
#define ELF_HELPERS_H

#include <memory>
#include <stdexcept>
#include <string>

#include <gelf.h>
#include <libelf.h>

namespace elf {
    class exception : public std::runtime_error {
    public:
        exception (char const * prefix, int err);
        explicit exception (char const * message);

    private:
        static std::string message (char const * prefix, int err);
    };


    typedef std::unique_ptr<Elf, decltype (&elf_end)> elf_ptr;
    elf_ptr begin (int fd, Elf_Cmd cmd, Elf * ref = nullptr);
    elf_ptr begin (FILE * const file, Elf_Cmd cmd, Elf * ref = nullptr);


    // ELF_C_NULL : The library will recalculate structural information flagging modified structures
    // with the
    //              ELF_F_DIRTY flag, but will not write back data to the underlying file image.
    // ELF_C_WRITE: The library will recalculate structural information and will also write the new
    // image to
    //              the underlying file.
    void update (Elf * elf, Elf_Cmd cmd);

    inline void update (elf::elf_ptr const & elf, Elf_Cmd cmd) {
        return update (elf.get (), cmd);
    }


    Elf_Scn * newscn (Elf * elf);
    Elf_Data * newdata (Elf_Scn * section);

    Elf32_Ehdr * newehdr32 (Elf * elf);
    Elf32_Shdr * getshdr32 (Elf_Scn * section);

    Elf64_Ehdr * newehdr64 (Elf * elf);
    Elf64_Shdr * getshdr64 (Elf_Scn * section);

    char const * strptr (Elf * const elf, std::size_t section, std::size_t offset);
    Elf_Scn * getscn (Elf * const elf, std::size_t index);
}

namespace gelf {
    GElf_Ehdr getehdr (Elf * const elf);
    void update_ehdr (Elf * const elf, GElf_Ehdr const & ehdr);

    GElf_Shdr getshdr (Elf_Scn * const section);
    void update_shdr (Elf_Scn * const section, GElf_Shdr const & shdr);

    GElf_Sym getsym (Elf_Data * const data, unsigned section_index);
    GElf_Sym getsym (Elf_Scn * const symbol_table, unsigned symbol_index);
    void update_sym (Elf_Data * const data, int index, GElf_Sym const & src);
}
#endif // ELF_HELPERS_H
// eof elf_helpers.h
