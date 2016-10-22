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

#ifndef ELF_ENUMERATOR_HPP
#define ELF_ENUMERATOR_HPP

#include <boost/filesystem.hpp>

class comdat_scanner_base;
class updater;

void enumerate (int fd, boost::filesystem::path const & user_file_path,
                comdat_scanner_base * const scanner, updater * const progress);

void enumerate (boost::filesystem::path const & path,
                boost::filesystem::path const & user_file_path, comdat_scanner_base * const scanner,
                updater * const progress);


/// Returns the number of members in the ELF container
unsigned members (int fd, boost::filesystem::path const & user_file_path);
unsigned members (boost::filesystem::path const & path,
                  boost::filesystem::path const & user_file_path);

#endif // ELF_ENUMERATOR_HPP
// eof elf_enumerator.hpp
