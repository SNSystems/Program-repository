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

#include "elf_helpers.hpp"

#include <cassert>
#include <sstream>

namespace {
    // check_version
    // ~~~~~~~~~~~~~~
    void check_version () {
        (void) ::elf_version (EV_NONE);
        if (::elf_version (EV_CURRENT) == EV_NONE) {
            throw std::runtime_error ("libelf.a is out of date");
        }
    }
}

namespace elf {
    // *************
    // * exception *
    // *************
    exception::exception (char const * prefix, int err)
            : std::runtime_error (message (prefix, err)) {}
    exception::exception (char const * message)
            : std::runtime_error (message) {}

    std::string exception::message (char const * prefix, int err) {
        assert (err != 0);
        std::ostringstream str;
        str << prefix << " failed:" << elf_errmsg (err);
        return str.str ();
    }


    // begin
    // ~~~~~
    elf_ptr begin (int fd, Elf_Cmd cmd, Elf * ref) {
        check_version ();
        ::elf_errno (); // force the ELF error code to be zero.
        elf_ptr result{::elf_begin (fd, cmd, ref), &::elf_end};
        if (auto const err = ::elf_errno ()) {
            throw exception ("elf_begin", err);
        }
        return result;
    }
    elf_ptr begin (FILE * const file, Elf_Cmd cmd, Elf * ref) {
        return begin (fileno (file), cmd, ref);
    }



    void update (Elf * const elf, Elf_Cmd cmd) {
        if (::elf_update (elf, cmd) < 0) {
            throw exception ("elf_update", ::elf_errno ());
        }
    }



    Elf_Scn * newscn (Elf * const elf) {
        Elf_Scn * const section = ::elf_newscn (elf);
        if (section == nullptr) {
            throw exception ("elf_newscn", ::elf_errno ());
        }
        return section;
    }

    Elf_Data * newdata (Elf_Scn * const section) {
        Elf_Data * const data = elf_newdata (section);
        if (data == nullptr) {
            throw exception ("elf_newdata", ::elf_errno ());
        }
        return data;
    }

    char const * strptr (Elf * const elf, std::size_t section, std::size_t offset) {
        char const * name = ::elf_strptr (elf, section, offset);
        if (name == nullptr) {
            throw exception ("elf_strptr", ::elf_errno ());
        }
        return name;
    }

    Elf_Scn * getscn (Elf * const elf, std::size_t index) {
        Elf_Scn * const section = ::elf_getscn (elf, index);
        if (section == nullptr) {
            throw exception ("elf_strptr", ::elf_errno ());
        }
        return section;
    }


    // new/get ehdr32
    // ~~~~~~~~~~~~~~
    Elf32_Ehdr * newehdr32 (Elf * const elf) {
        Elf32_Ehdr * const ehdr = ::elf32_newehdr (elf);
        if (ehdr == nullptr) {
            throw exception ("elf32_newehdr", ::elf_errno ());
        }
        return ehdr;
    }
    Elf32_Shdr * getshdr32 (Elf_Scn * const section) {
        Elf32_Shdr * const shdr = ::elf32_getshdr (section);
        if (shdr == nullptr) {
            throw exception ("elf32_getshdr", ::elf_errno ());
        }
        return shdr;
    }


    // new/get ehdr64
    // ~~~~~~~~~~~~~~
    Elf64_Ehdr * newehdr64 (Elf * const elf) {
        Elf64_Ehdr * const ehdr = ::elf64_newehdr (elf);
        if (ehdr == nullptr) {
            throw exception ("elf64_newehdr () failed: ", ::elf_errno ());
        }
        return ehdr;
    }
    Elf64_Shdr * getshdr64 (Elf_Scn * const section) {
        Elf64_Shdr * const shdr = elf64_getshdr (section);
        if (shdr == nullptr) {
            throw exception ("elf64_getshdr () failed: ", ::elf_errno ());
        }
        return shdr;
    }
}

namespace gelf {
    // get/update ehdr
    // ~~~~~~~~~~~~~~~
    GElf_Ehdr getehdr (Elf * const elf) {
        assert (elf != nullptr);
        GElf_Ehdr result;
        GElf_Ehdr * const res = ::gelf_getehdr (elf, &result);
        if (res == nullptr) {
            throw elf::exception ("gelf_update_ehdr", ::elf_errno ());
        }
        if (res != &result) {
            throw elf::exception ("gelf_update_ehdr returned an unexpected error");
        }
        return result;
    }
    void update_ehdr (Elf * const elf, GElf_Ehdr const & ehdr) {
        assert (elf != nullptr);
        int const ok = ::gelf_update_ehdr (elf, const_cast<GElf_Ehdr *> (&ehdr));
        if (!ok) {
            throw elf::exception ("gelf_update_ehdr", ::elf_errno ());
        }
    }


    // get/update shdr
    // ~~~~~~~~~~~~~~~
    GElf_Shdr getshdr (Elf_Scn * const section) {
        assert (section != nullptr);
        GElf_Shdr out;
        GElf_Shdr * const res = ::gelf_getshdr (section, &out);
        if (res == nullptr) {
            throw elf::exception ("gelf_getshdr", ::elf_errno ());
        }
        if (res != &out) {
            throw elf::exception ("gelf_getshdr returned an unexpected error");
        }
        return out;
    }
    void update_shdr (Elf_Scn * const section, GElf_Shdr const & shdr) {
        assert (section != nullptr);
        int const ok = ::gelf_update_shdr (section, const_cast<GElf_Shdr *> (&shdr));
        if (!ok) {
            throw elf::exception ("gelf_update_shdr", ::elf_errno ());
        }
    }


    // get/update sym
    // ~~~~~~~~~~~~~~
    GElf_Sym getsym (Elf_Data * const data, unsigned section_index) {
        assert (data != nullptr);
        GElf_Sym out;
        GElf_Sym * const res = ::gelf_getsym (data, section_index, &out);
        if (res == nullptr) {
            throw elf::exception ("gelf_getsym", ::elf_errno ());
        }
        if (res != &out) {
            throw elf::exception ("gelf_getsym returned an unexpected error");
        }
        return out;
    }
    GElf_Sym getsym (Elf_Scn * const symbol_table, unsigned symbol_index) {
        assert (symbol_table != nullptr);
        Elf_Data * const data = ::elf_getdata (symbol_table, nullptr);
        if (data == nullptr) {
            throw elf::exception ("elf_getdata", ::elf_errno ());
        }
        return gelf::getsym (data, symbol_index);
    }
    void update_sym (Elf_Data * const data, int index, GElf_Sym const & src) {
        assert (data != nullptr);
        int const ok = ::gelf_update_sym (data, index, const_cast<GElf_Sym *> (&src));
        if (!ok) {
            throw elf::exception ("gelf_update_sym", ::elf_errno ());
        }
    }
} // namespace elf
// eof elf_helpers.cpp
