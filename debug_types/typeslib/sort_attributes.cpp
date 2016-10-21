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

#include "sort_attributes.hpp"

#include <array>
#include <limits>

#include <dwarf.h>
#include <libdwarf.h>

namespace {

    /// Creates and returns a container whose indices are the attribute tags and whose value
    /// represents the sort order of that attribute (smaller values come before larger). Attributes
    /// not to be
    /// considered have the value maxint.

    auto ordered_attributes () -> details::attribute_order {
        using namespace details;

        static constexpr std::array<Dwarf_Half, 48> order{{
            DW_AT_name,
            DW_AT_accessibility,
            DW_AT_address_class,
            DW_AT_allocated,
            DW_AT_artificial,
            DW_AT_associated,
            DW_AT_binary_scale,
            DW_AT_bit_offset,
            DW_AT_bit_size,
            DW_AT_bit_stride,
            DW_AT_byte_size,
            DW_AT_byte_stride,
            DW_AT_const_expr,
            DW_AT_const_value,
            DW_AT_containing_type,
            DW_AT_count,
            DW_AT_data_bit_offset,
            DW_AT_data_location,
            DW_AT_data_member_location,
            DW_AT_decimal_scale,
            DW_AT_decimal_sign,
            DW_AT_default_value,
            DW_AT_digit_count,
            DW_AT_discr,
            DW_AT_discr_list,
            DW_AT_discr_value,
            DW_AT_encoding,
            DW_AT_enum_class,
            DW_AT_endianity,
            DW_AT_explicit,
            DW_AT_is_optional,
            DW_AT_location,
            DW_AT_lower_bound,
            DW_AT_mutable,
            DW_AT_ordering,
            DW_AT_picture_string,
            DW_AT_prototyped,
            DW_AT_small,
            DW_AT_segment,
            DW_AT_string_length,
            DW_AT_threads_scaled,
            DW_AT_upper_bound,
            DW_AT_use_location,
            DW_AT_use_UTF8,
            DW_AT_variable_parameter,
            DW_AT_virtuality,
            DW_AT_visibility,
            DW_AT_vtable_elem_location,
        }};

        typedef attribute_order::value_type value_type;
        static_assert (std::numeric_limits<value_type>::max () >=
                           std::numeric_limits<decltype (order)::value_type>::max (),
                       "attribute_order::value_type is not wide enough");
        static_assert (std::numeric_limits<value_type>::min () <=
                           std::numeric_limits<decltype (order)::value_type>::min (),
                       "attribute_order::value_type is not wide enough");

        auto const max = *std::max_element (std::begin (order), std::end (order));
        attribute_order att2 (max + 1U,
                              default_priority); // initialize and fill the vector with maxint
        auto priority = value_type{0};
        for (Dwarf_Half at : order) {
            att2[at] = priority++;
        }
        return att2;
    }
}

namespace details {
    attribute_order const order = ordered_attributes ();
    std::size_t const order_size = order.size ();
}

// eof sort_attributes.cpp
