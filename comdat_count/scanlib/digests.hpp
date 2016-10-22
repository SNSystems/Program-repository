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

#ifndef SCANLIB_DIGEST_HPP
#define SCANLIB_DIGEST_HPP

#include "elf_helpers.hpp"
#include "md5_context.h"
#include <list>
#include <mutex>

class digests {
public:
    void add_elf (Elf * const elf);
    void add_elf (elf::elf_ptr const & elf);

    /// Returns the final MD5 digest for all of the inputs. This is produced by taking the (sorted)
    /// collections of MD5s from the individual inputs and hashing them together.
    auto final () -> md5::digest;

    static auto md5 (Elf * const elf) -> md5::digest;
    static auto md5 (elf::elf_ptr const & elf) -> md5::digest;

private:
    std::mutex hashes_lock_;
    std::list<md5::digest> hashes_;
};

#endif // SCANLIB_DIGEST_HPP
// digests.hpp
