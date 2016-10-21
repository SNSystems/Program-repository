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

#include "scan_type.hpp"
#include "print.hpp"

dwarf::die_ptr die_referenced_by_attribute (dwarf::attribute const & attribute) {
    switch (attribute.what_form ()) {
    case DW_FORM_ref_addr:
    case DW_FORM_ref1:
    case DW_FORM_ref2:
    case DW_FORM_ref4:
    case DW_FORM_ref8:
    case DW_FORM_ref_udata:
        return attribute.get_debug ()->offset_to_die (attribute.ref ());
    case DW_FORM_ref_sig8:
        print_cerr ("DIE has a type reference via unsupported DW_FORM_ref_sig8");
        std::abort ();
    }
    return dwarf::die_ptr{};
}

dwarf::die_ptr type_die_referenced_by_attribute (dwarf::attribute const & attribute) {
    if (dwarf::die_ptr ref_die = die_referenced_by_attribute (attribute)) {
        if (attribute.get_debug ()->is_type_die (ref_die)) {
            return ref_die;
        }
    }
    return dwarf::die_ptr{};
}

// eof scan_type.cpp
