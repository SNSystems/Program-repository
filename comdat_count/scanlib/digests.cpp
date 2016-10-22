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
#include <cassert>
#include <libelf.h>

// add_elf
// ~~~~~~~
void digests::add_elf (Elf * const elf) {
    auto const digest = this->md5 (elf);
    std::lock_guard<std::mutex> guard (hashes_lock_);
    hashes_.push_back (digest);
}

void digests::add_elf (elf::elf_ptr const & elf) {
    return add_elf (elf.get ());
}


// md5 [static]
// ~~~
auto digests::md5 (Elf * const elf) -> md5::digest {
    auto size = std::size_t{0};
    void const * contents = elf_rawfile (elf, &size);
    assert (size > 0 && contents != nullptr);

    static char const begin[] = "Bgn";
    static char const end[] = "End";

    md5::context context;
    context.update (begin, sizeof (begin));
    context.update (contents, size);
    context.update (end, sizeof (end));
    return context.finalize ();
}

auto digests::md5 (elf::elf_ptr const & elf) -> md5::digest {
    return md5 (elf.get ());
}


// final
// ~~~~~
auto digests::final () -> md5::digest {
    std::lock_guard<std::mutex> guard (hashes_lock_);
    hashes_.sort ([](md5::digest const & a, md5::digest const & b) {
        assert (a.size () == b.size ());
        auto const size = a.size ();
        for (auto idx = 0U; idx < size; ++idx) {
            if (a[idx] != b[idx]) {
                return a[idx] < b[idx];
            }
        }
        return false;
    });
    md5::context context;
    for (auto const & d : hashes_) {
        context.update (d.data (), d.size ());
    }
    return context.finalize ();
}

// elf digests.cpp
