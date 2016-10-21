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

#include "visited_types.hpp"
#include "dwarf_helpers.hpp"
#include <cassert>

// (ctor)
// ~~~~~~
visited_types::visited_types (dwarf::debug * const debug)
        : debug_ (debug) {}

// find
// ~~~~
visited_types::const_iterator visited_types::find (Dwarf_Die die) const {
    auto const offset = debug_->die_to_offset (die);
    return v_.find (offset);
}

// add
// ~~~
unsigned visited_types::add (Dwarf_Die die) {
    auto it = this->find (die);
    if (it == std::end (v_)) {
        auto const offset = debug_->die_to_offset (die);
        v_[offset] = ++index_;
        return index_;
    } else {
        return it->second;
    }
}
// eof visited_types.cpp
