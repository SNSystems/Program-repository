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
    void check_version () {
        (void) ::elf_version (EV_NONE);
        if (::elf_version (EV_CURRENT) == EV_NONE) {
            throw elf::version_mismatch ();
        }
        if (auto const err = ::elf_errno ()) {
            throw elf::error ("elf_version", err);
        }
    }
}

namespace elf {
    // ********************
    // * version_mismatch *
    // ********************
    version_mismatch::version_mismatch ()
            : std::runtime_error ("libelf is out of date") {}


    // *********
    // * error *
    // *********
    // (ctor)
    // ~~~~~~
    error::error (char const * api, int err)
            : std::runtime_error (message (api, err)) {}

    // message
    // ~~~~~~~
    std::string error::message (char const * api, int err) {
        assert (api != nullptr && err != 0);

        std::ostringstream str;
        str << api << ": " << elf_errmsg (err) << " (" << err << ')';
        return str.str ();
    }


    // ************
    // * make_elf *
    // ************
    elf_ptr make_elf (void const * image, std::size_t size) {
        check_version ();
        auto imagec = const_cast<char *> (static_cast<char const *> (image));
        elf_ptr resl{::elf_memory (imagec, size), &::elf_end};
        if (auto const err = ::elf_errno ()) {
            throw error ("elf_memory", err);
        }
        return resl;
    }

    elf_ptr make_elf (file_opener const & fd, Elf_Cmd cmd, Elf * const ref) {
        check_version ();
        elf_ptr resl{::elf_begin (fd.get (), cmd, ref), &::elf_end};
        if (auto const err = ::elf_errno ()) {
            throw error ("elf_begin", err);
        }
        return resl;
    }

    // **********
    // * is_elf *
    // **********
    bool is_elf (elf_ptr const & elf) {
        assert (::elf_errno () == 0);
        ::elf32_getehdr (elf.get ());
        if (::elf_errno () == 0) {
            return true;
        }
        ::elf64_getehdr (elf.get ());
        return ::elf_errno () == 0;
    }

    // ********
    // * kind *
    // ********
    Elf_Kind kind (elf_ptr const & e) {
        auto const resl = ::elf_kind (e.get ());
        if (auto const err = ::elf_errno ()) {
            throw error ("elf_kind", err);
        }
        return resl;
    }
} // namespace elf
// eof elf_helpers.cpp
