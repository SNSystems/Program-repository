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
#include <memory>
#include <vector>

#include "dwarf_helpers.hpp"
#include "mock_debug.hpp"
#include <gmock/gmock.h>



TEST (SortAttributes, Empty) {
    mock_debug debug;
    std::vector<dwarf::attribute> attributes;
    decltype (attributes)::iterator result =
        sort_attributes (std::begin (attributes), std::end (attributes));
    EXPECT_EQ (std::end (attributes), result);
}


TEST (SortAttributes, OneName) {
    using ::testing::Return;
    mock_debug debug;

    std::array<Dwarf_Attribute_s, 1> raw_attributes{{
        {1, 1},
    }}; // FIXME:
    std::array<dwarf::attribute, 1> attributes{{
        {&debug, &raw_attributes[0]},
    }};
    EXPECT_CALL (debug, attribute_what_attr (&raw_attributes[0]))
        .WillRepeatedly (Return (DW_AT_name));

    auto const result = sort_attributes (std::begin (attributes), std::end (attributes));

    EXPECT_EQ (&raw_attributes[0], attributes[0].get_attribute ());

    // DW_AT_name is included in the list of prioritized attributes, so sort_attributes() should
    // return
    // the end iterator.
    EXPECT_EQ (std::end (attributes), result);
}


TEST (SortAttributes, ThreeAttributes) {
    using ::testing::Return;
    mock_debug debug;

    // Simulate three attributes in the order DW_AT_decl_file, DW_AT_name, DW_AT_byte_size. The sort
    // process
    // produce the order: DW_AT_name, DW_AT_byte_size, DW_AT_declfile.
    std::array<Dwarf_Attribute_s, 3> raw_attributes{{
        {1, 1}, {1, 1}, {1, 1},
    }}; // FIXME:
    std::array<dwarf::attribute, 3> attributes{{
        {&debug, &raw_attributes[0]}, {&debug, &raw_attributes[1]}, {&debug, &raw_attributes[2]},
    }};
    EXPECT_CALL (debug, attribute_what_attr (&raw_attributes[0]))
        .WillRepeatedly (Return (DW_AT_decl_file));
    EXPECT_CALL (debug, attribute_what_attr (&raw_attributes[1]))
        .WillRepeatedly (Return (DW_AT_name));
    EXPECT_CALL (debug, attribute_what_attr (&raw_attributes[2]))
        .WillRepeatedly (Return (DW_AT_byte_size));

    auto const result = sort_attributes (std::begin (attributes), std::end (attributes));

    EXPECT_EQ (&raw_attributes[1], attributes[0].get_attribute ());
    EXPECT_EQ (&raw_attributes[2], attributes[1].get_attribute ());
    EXPECT_EQ (&raw_attributes[0], attributes[2].get_attribute ());

    auto expected = std::begin (attributes);
    std::advance (expected, 2);
    EXPECT_EQ (expected, result);
}
