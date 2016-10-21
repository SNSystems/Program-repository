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

#include "names.hpp"
#include <libdwarf.h>
#include <sstream>

namespace {
    template <typename GetFunction>
    std::string get_name (Dwarf_Half val, GetFunction function, char const * name) {
        char const * str{nullptr};
        int res = function (val, &str);
        if (res == DW_DLV_OK) {
            return {str};
        } else {
            std::ostringstream os;
            os << "<unknown " << name << " value " << val << ">";
            return os.str ();
        }
    }
}

std::string get_TAG_name (Dwarf_Half val) {
    return get_name (val, dwarf_get_TAG_name, "TAG");
}

std::string get_FORM_name (Dwarf_Half val) {
    return get_name (val, dwarf_get_FORM_name, "FORM");
}

std::string get_AT_name (Dwarf_Half val) {
    return get_name (val, dwarf_get_AT_name, "AT");
}

// eof names.cpp
