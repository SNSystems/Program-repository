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

#include "leb128.hpp"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

#include <gmock/gmock.h>

TEST (Leb128, EncodeUlebExampleFromDwarf4Figure22) {
    std::uint8_t bytes[6];
    std::vector<std::uint8_t> buffer;
    buffer.reserve (5);

    // From the DWARF 4 spec: figure 22.
    buffer.clear ();
    encode_uleb128 (2, std::back_inserter (buffer));
    ASSERT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{2}));
    EXPECT_EQ (encode_uleb128 (2, bytes), bytes + 1); // check the return value

    buffer.clear ();
    encode_uleb128 (127, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{127}));

    buffer.clear ();
    encode_uleb128 (128, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0 + 0x80}, std::uint8_t{1}));
    EXPECT_EQ (encode_uleb128 (128, bytes), bytes + 2); // check the return value

    buffer.clear ();
    encode_uleb128 (129, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{1 + 0x80}, std::uint8_t{1}));

    buffer.clear ();
    encode_uleb128 (130, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{2 + 0x80}, std::uint8_t{1}));

    buffer.clear ();
    encode_uleb128 (12857, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{57 + 0x80}, std::uint8_t{100}));

    // From the Wikipedia article on Leb128
    // https://en.wikipedia.org/wiki/LEB128#Unsigned_LEB128
    buffer.clear ();
    encode_uleb128 (624485, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0xE5}, std::uint8_t{0x8E},
                                                 std::uint8_t{0x26}));

    // Miscellaneous
    buffer.clear ();
    encode_uleb128 (0, std::back_inserter (buffer));
    ASSERT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0}));
    EXPECT_EQ (encode_uleb128 (0, bytes), bytes + 1); // check the return value

    buffer.clear ();
    encode_uleb128 (0xffffffff, std::back_inserter (buffer));
    EXPECT_THAT (buffer,
                 ::testing::ElementsAre (std::uint8_t{0xff}, std::uint8_t{0xff}, std::uint8_t{0xff},
                                         std::uint8_t{0xff}, std::uint8_t{0x0f}));
    EXPECT_EQ (encode_uleb128 (0xffffffff, bytes), bytes + 5); // check the return value
}


TEST (Leb128, EncodeSlebExampleFromDwarf4Figure22) {
    std::uint8_t bytes[6];
    std::vector<std::uint8_t> buffer;
    buffer.reserve (3);

    buffer.clear ();
    encode_sleb128 (0, std::back_inserter (buffer));
    ASSERT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0}));
    EXPECT_EQ (encode_sleb128 (0, bytes), bytes + 1); // check the return value

    buffer.clear ();
    encode_sleb128 (2, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{2}));
    EXPECT_EQ (encode_sleb128 (2, bytes), bytes + 1); // check the return value

    buffer.clear ();
    encode_sleb128 (-2, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0x7e}));

    buffer.clear ();
    encode_sleb128 (127, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{127 + 0x80}, std::uint8_t{0}));

    buffer.clear ();
    encode_sleb128 (-127, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{1 + 0x80}, std::uint8_t{0x7f}));

    buffer.clear ();
    encode_sleb128 (128, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0 + 0x80}, std::uint8_t{1}));

    buffer.clear ();
    encode_sleb128 (-128, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0 + 0x80}, std::uint8_t{0x7f}));

    buffer.clear ();
    encode_sleb128 (129, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{1 + 0x80}, std::uint8_t{1}));

    buffer.clear ();
    encode_sleb128 (-129, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0x7f + 0x80}, std::uint8_t{0x7e}));

    // Example from the Wikipedia article on Leb128
    // https://en.wikipedia.org/wiki/LEB128#Signed_LEB128
    buffer.clear ();
    encode_sleb128 (-624485, std::back_inserter (buffer));
    EXPECT_THAT (buffer, ::testing::ElementsAre (std::uint8_t{0x9b}, std::uint8_t{0xf1},
                                                 std::uint8_t{0x59}));
}
