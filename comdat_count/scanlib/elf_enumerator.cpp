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

#include "elf_enumerator.hpp"

#include <cassert>
#include <cstdio>
#include <exception>
#include <memory>
#include <sstream>

#include "comdat_scanner.hpp"
#include "consumer.hpp"
#include "elf_helpers.hpp"
#include "print.hpp"
#include "progress.hpp"

namespace {
    // is elf
    // ~~~~~~
    bool is_elf (Elf * const elf) {
        assert (::elf_errno () == 0);
        ::elf32_getehdr (elf);
        if (::elf_errno () == 0) {
            return true;
        }
        ::elf64_getehdr (elf);
        return ::elf_errno () == 0;
    }


    // open file
    // ~~~~~~~~~~
    typedef std::unique_ptr<FILE, decltype (&std::fclose)> file_ptr;

    file_ptr open_file (boost::filesystem::path const & path) {
        FILE * const f = ::fopen (path.string ().c_str (), "rb");
        if (f == nullptr) {
            std::ostringstream str;
            str << "Could not open " << path;
            throw std::system_error (errno, std::generic_category (), str.str ());
        }

        return {f, &std::fclose};
    }
}


unsigned members (int fd, boost::filesystem::path const & user_file_path) {
    Elf_Cmd cmd = ELF_C_READ;
    // FIXME: factor out the duplicated enumeration code.
    elf::elf_ptr archive = elf::begin (fd, cmd, nullptr);
    if (archive.get () == nullptr) {
        std::ostringstream str;
        str << "elf_begin () for " << user_file_path << " failed";
        throw elf::exception (str.str ().c_str (), elf_errno ());
    }

    auto members = 0U;
    for (;;) {
        elf::elf_ptr elf = elf::begin (fd, cmd, archive.get ());
        Elf * const elfp = elf.get ();
        if (elfp == nullptr) {
            break;
        }
        ++members;
        cmd = elf_next (elfp);
    }
    return members;
}


unsigned members (boost::filesystem::path const & path,
                  boost::filesystem::path const & user_file_path) {
    auto file = open_file (path);
    return members (fileno (file.get ()), user_file_path);
}


void enumerate (int fd, boost::filesystem::path const & user_file_path,
                comdat_scanner_base * const scanner, updater * const progress) {
    assert (scanner != nullptr);

    // FIXME: factor out the duplicated enumeration code.
    // If we're processing an archive, then we need to loop through
    // the files that it contains.
    Elf_Cmd cmd = ELF_C_READ;
    elf::elf_ptr archive (nullptr, &::elf_end);
    bool skip = false;
    try {
        archive = elf::begin (fd, cmd, nullptr);
    } catch (elf::exception const &) {
        skip = true;
    }
    if (skip) {
        scanner->skip (user_file_path, nullptr);
        return;
    }

    if (progress != nullptr) {
        unsigned m = members (fd, user_file_path);
        assert (m >= 1);
        progress->total_incr (m - 1);
    }

    for (;;) {
        if (cmd == ELF_C_NULL) {
            // The last archive member (or the sole object file) has
            // been processed.
            break;
        }
        elf::elf_ptr elf = elf::begin (fd, cmd, archive.get ());
        Elf * const elfp = elf.get ();
        if (is_elf (elfp)) {
            // It's an ELF file, so scan it...
            scanner->scan (user_file_path, elfp);
        } else {
            scanner->skip (user_file_path, elfp);
        }

        if (progress != nullptr) {
            progress->completed_incr ();
        }

        // If we're processing an archive, move to the next file.
        cmd = elf_next (elfp);
    }
}

void enumerate (boost::filesystem::path const & path,
                boost::filesystem::path const & user_file_path, comdat_scanner_base * const scanner,
                updater * const progress) {

    assert (scanner != nullptr);
    auto file = open_file (path);
    enumerate (fileno (file.get ()), user_file_path, scanner, progress);
}

// eof elf_numerator.cpp
