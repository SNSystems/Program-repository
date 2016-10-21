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

#ifndef ELF_HELPERS_HPP
#define ELF_HELPERS_HPP

#include <memory>
#include <stdexcept>

#include "dt_config.h"
#if HAVE_LIBELF_H
#include <libelf.h>
#elif HAVE_LIBELF_LIBELF_H
#include <libelf/libelf.h>
#else
#error "Need libelf.h!"
#endif

#include "file_opener.hpp"

namespace elf {
    class version_mismatch : public std::runtime_error {
    public:
        version_mismatch ();
    };

    class error : public std::runtime_error {
    public:
        error (char const * api, int err);

    private:
        static std::string message (char const * api, int err);
    };


    using elf_ptr = std::unique_ptr<Elf, decltype (&::elf_end)>;

    elf_ptr make_elf (void const * image, std::size_t size);
    elf_ptr make_elf (file_opener const & fd, Elf_Cmd cmd, Elf * ref = nullptr);

    bool is_elf (elf_ptr const & elf);
    Elf_Kind kind (elf_ptr const & e);
}

#endif // ELF_HELPERS_HPP
// eof elf_helpers.hpp
