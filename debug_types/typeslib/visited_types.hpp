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

#ifndef VISITED_TYPES_HPP
#define VISITED_TYPES_HPP

#include <libdwarf.h>
#include <unordered_map>

namespace dwarf {
    class debug;
}

class visited_types {
    using container = std::unordered_map<Dwarf_Off, unsigned>;

public:
    using const_iterator = container::const_iterator;

    explicit visited_types (dwarf::debug * const debug);

    const_iterator begin () const {
        return v_.cbegin ();
    }
    const_iterator end () const {
        return v_.cend ();
    }

    const_iterator find (Dwarf_Die die) const;
    unsigned add (Dwarf_Die die);

private:
    dwarf::debug * const debug_;
    unsigned index_ = 0;
    container v_;
};

#endif // VISITED_TYPES_HPP
// eof visited_types.hpp
