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

#include "cu_iterator.hpp"

cu_iterator::cu_iterator (dwarf::debug * const debug, Dwarf_Die die)
        : debug_ (debug)
        , next_cu_ ()
        , die_ (die) {}

cu_iterator::cu_iterator (dwarf::debug * const debug)
        : debug_ (debug)
        , next_cu_ ()
        , die_ (nullptr) {
    this->increment ();
}



dwarf::die_ptr cu_iterator::next_cu_info::next (dwarf::debug * const debug) {
    if (debug->next_cu_header (&cu_header_length, &version_stamp, &abbrev_offset, &address_size,
                               &next_cu_offset)) {
        return debug->siblingof (nullptr);
    }
    return debug->as_die_ptr (nullptr);
}

// eof cu_iterator.cpp
