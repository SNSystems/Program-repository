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

#include "make_elf.hpp"
#include <cassert>

namespace {
    template <typename Header>
    void populate_header (Header * const h, endian e) {
        h->e_ident[EI_DATA] = (e == endian::big) ? ELFDATA2MSB : ELFDATA2LSB;
        h->e_machine = EM_ARM;
        h->e_type    = ET_REL;
        h->e_version = EV_CURRENT;
    }
}

template <>
    elf::elf_ptr make_elf <endian::big, 32U> (int fd) {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_WRITE);
        populate_header (elf::newehdr32 (elf.get ()), endian::big);
        return elf;
    }
template <>
    elf::elf_ptr make_elf <endian::big, 32U> (FILE * const file) {
        return make_elf <endian::big, 32U> (fileno (file));
    }


template <>
    elf::elf_ptr make_elf <endian::big, 64U> (int fd) {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_WRITE);
        populate_header (elf::newehdr64 (elf.get ()), endian::big);
        return elf;
    }
template <>
    elf::elf_ptr make_elf <endian::big, 64U> (FILE * const file) {
        return make_elf <endian::big, 64U> (fileno (file));
    }


template <>
    elf::elf_ptr make_elf <endian::little, 32U> (int fd) {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_WRITE);
        populate_header (elf::newehdr32 (elf.get ()), endian::little);
        return elf;
    }
template <>
    elf::elf_ptr make_elf <endian::little, 32U> (FILE * const file) {
        return make_elf <endian::little, 32U> (fileno (file));
    }


template <>
    elf::elf_ptr make_elf <endian::little, 64U> (int fd) {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_WRITE);
        populate_header (elf::newehdr64 (elf.get ()), endian::little);
        return elf;
    }
template <>
    elf::elf_ptr make_elf <endian::little, 64U> (FILE * const file) {
        return make_elf <endian::little, 64U> (fileno (file));
    }

// elf unittest/make_elf.cpp
