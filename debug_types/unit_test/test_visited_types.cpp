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

#include <iterator>

#include "mock_debug.hpp"
#include <gmock/gmock.h>

TEST (VisitedTypes, Empty) {
    mock_debug debug;
    visited_types v (&debug);
    EXPECT_EQ (0U, std::distance (v.begin (), v.end ()));
}

TEST (VisitedTypes, SingleDie) {
    using ::testing::Return;
    ::testing::NiceMock<mock_debug> debug;
    visited_types v (&debug);

    dwarf::die_ptr d1 = debug.new_die ();
    Dwarf_Die d1p = d1.get ();
    constexpr Dwarf_Off offset = 127;
    ON_CALL (debug, die_to_offset (d1p)).WillByDefault (Return (offset));

    // The DIE is not present in the list of types.
    EXPECT_EQ (v.end (), v.find (d1p));

    // Add it.
    unsigned index = v.add (d1p);
    EXPECT_EQ (1U, index);

    // Check the presence of the DIE and that it has been given the correct index.
    {
        visited_types::const_iterator it = v.find (d1p);
        EXPECT_NE (v.end (), it);
        EXPECT_EQ (visited_types::const_iterator::value_type (offset, 1U), *it);
    }
    // Try to add it again.
    index = v.add (d1p);
    EXPECT_EQ (1U, index);
    // Check that we get back the original index.
    EXPECT_EQ (visited_types::const_iterator::value_type (offset, 1U), *v.find (d1p));
}

TEST (VisitedTypes, TwoDies) {
    using ::testing::Return;
    ::testing::NiceMock<mock_debug> debug;
    visited_types v (&debug);

    dwarf::die_ptr d1 = debug.new_die ();
    dwarf::die_ptr d2 = debug.new_die ();
    Dwarf_Die d1p = d1.get ();
    Dwarf_Die d2p = d2.get ();
    constexpr Dwarf_Off offset1 = 93;
    constexpr Dwarf_Off offset2 = 127;
    ON_CALL (debug, die_to_offset (d1p)).WillByDefault (Return (offset1));
    ON_CALL (debug, die_to_offset (d2p)).WillByDefault (Return (offset2));

    // Add it.
    unsigned index1 = v.add (d1p);
    EXPECT_EQ (1U, index1);
    unsigned index2 = v.add (d2p);
    EXPECT_EQ (2U, index2);
}
// eof test_visited_types.cpp
