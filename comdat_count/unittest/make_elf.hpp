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

#ifndef UNITTEST_MAKE_ELF_HPP
#define UNITTEST_MAKE_ELF_HPP

#include "elf_helpers.hpp"
enum class endian { big, little };

template <endian Endian, unsigned Bits>
    auto make_elf (int fd) -> elf::elf_ptr;
template <endian Endian, unsigned Bits>
    auto make_elf (FILE * const file) -> elf::elf_ptr;

template <> auto make_elf <endian::big   , 32U> (int fd) -> elf::elf_ptr;
template <> auto make_elf <endian::big   , 64U> (int fd) -> elf::elf_ptr;
template <> auto make_elf <endian::little, 32U> (int fd) -> elf::elf_ptr;
template <> auto make_elf <endian::little, 64U> (int fd) -> elf::elf_ptr;

template <> auto make_elf <endian::big   , 32U> (FILE * const file) -> elf::elf_ptr;
template <> auto make_elf <endian::big   , 64U> (FILE * const file) -> elf::elf_ptr;
template <> auto make_elf <endian::little, 32U> (FILE * const file) -> elf::elf_ptr;
template <> auto make_elf <endian::little, 64U> (FILE * const file) -> elf::elf_ptr;

inline auto make_be32_elf (int fd) -> elf::elf_ptr { return make_elf <endian::big   , 32U> (fd); }
inline auto make_le32_elf (int fd) -> elf::elf_ptr { return make_elf <endian::little, 32U> (fd); }
inline auto make_be64_elf (int fd) -> elf::elf_ptr { return make_elf <endian::big   , 64U> (fd); }
inline auto make_le64_elf (int fd) -> elf::elf_ptr { return make_elf <endian::little, 64U> (fd); }

inline auto make_be32_elf (FILE * const file) -> elf::elf_ptr { return make_elf <endian::big   , 32U> (file); }
inline auto make_le32_elf (FILE * const file) -> elf::elf_ptr { return make_elf <endian::little, 32U> (file); }
inline auto make_be64_elf (FILE * const file) -> elf::elf_ptr { return make_elf <endian::big   , 64U> (file); }
inline auto make_le64_elf (FILE * const file) -> elf::elf_ptr { return make_elf <endian::little, 64U> (file); }

#endif // UNITTEST_MAKE_ELF_HPP
// unittest/make_elf.hpp
