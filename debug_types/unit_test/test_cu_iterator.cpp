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
#include "mock_debug.hpp"
#include <gmock/gmock.h>

TEST (CuIterator, NoCUs) {
    using ::testing::_;
    using ::testing::Return;

    mock_debug debug;
    EXPECT_CALL (debug, next_cu_header (_, _, _, _, _)).Times (1).WillOnce (Return (true));
    EXPECT_CALL (debug, get_siblingof (nullptr)).Times (1);

    auto count = 0U;
    for (auto first = cu_iterator (&debug), last = cu_iterator (&debug, nullptr); first != last;
         ++first) {
        ++count;
    }
    EXPECT_EQ (0U, count);
}


TEST (CuIterator, OneCU) {
    using ::testing::_;
    using ::testing::Return;

    ::testing::NiceMock<mock_debug> debug;
    EXPECT_CALL (debug, next_cu_header (_, _, _, _, _))
        .Times (2)
        .WillOnce (Return (true))
        .WillOnce (Return (false));
    EXPECT_CALL (debug, get_siblingof (nullptr)).Times (1).WillOnce (Return (new Dwarf_Die_s));

    auto count = 0U;
    for (auto first = cu_iterator (&debug), last = cu_iterator (&debug, nullptr); first != last;
         ++first) {
        ++count;
    }
    EXPECT_EQ (1U, count);
}
