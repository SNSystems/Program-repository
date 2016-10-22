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

#include "digests.hpp"

#include <gmock/gmock.h>
#include "elf_helpers.hpp"

#include "make_elf.hpp"
#include "temporary_file.h"

// Compute the digest for the same file twice and make sure that we get the same answer for both.
TEST (Digests, IsConsistent) {
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());
    elf::update (make_elf <endian::big, 32U> (fd), ELF_C_WRITE);

    md5::digest result1;
    md5::digest result2;

    {
        auto elf = elf::begin (fd, ELF_C_READ);
        digests d;
        d.add_elf (elf.get ());
        result1 = d.final ();
    }
    {
        auto elf = elf::begin (fd, ELF_C_READ);
        digests d;
        d.add_elf (elf.get ());
        result2 = d.final ();
    }
    EXPECT_THAT (result1, ::testing::ContainerEq (result2));
}


TEST (Digests, OrderIsNotSignificant) {
    file_ptr f1 = temporary_file ();
    elf::update (make_elf <endian::big, 32U> (fileno (f1.get ())), ELF_C_WRITE);

    file_ptr f2 = temporary_file ();
    elf::update (make_elf <endian::little, 64> (fileno (f2.get ())), ELF_C_WRITE);

    digests d1;
    auto elf1 = elf::begin (fileno (f1.get ()), ELF_C_READ);
    auto elf2 = elf::begin (fileno (f2.get ()), ELF_C_READ);
    d1.add_elf (elf1);
    d1.add_elf (elf2);

    digests d2;
    d2.add_elf (elf2);
    d2.add_elf (elf1);

    EXPECT_THAT (d1.final (), ::testing::ContainerEq (d2.final ()));
}

// eof unittest/test_digests.cpp
