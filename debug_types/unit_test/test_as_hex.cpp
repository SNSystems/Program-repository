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

#include "as_hex.hpp"

#include <gmock/gmock.h>

TEST (as_hex, Zeroes) {
    std::ostringstream str;

    str.str ("");
    str << as_hex (std::uint8_t{0});
    EXPECT_EQ ("00", str.str ());

    str.str ("");
    str << as_hex (std::uint16_t{0});
    EXPECT_EQ ("0000", str.str ());

    str.str ("");
    str << as_hex (std::uint32_t{0});
    EXPECT_EQ ("00000000", str.str ());

    str.str ("");
    str << as_hex (std::uint64_t{0});
    EXPECT_EQ ("0000000000000000", str.str ());
}

TEST (as_hex, Big) {
    std::ostringstream str;

    str.str ("");
    str << as_hex (std::uint8_t{UINT8_C (0xFF)});
    EXPECT_EQ ("ff", str.str ());

    str.str ("");
    str << as_hex (std::uint16_t{UINT16_C (0xFFFF)});
    EXPECT_EQ ("ffff", str.str ());

    str.str ("");
    str << as_hex (std::uint32_t{UINT32_C (0xFFFFFFFF)});
    EXPECT_EQ ("ffffffff", str.str ());

    str.str ("");
    str << as_hex (std::uint64_t{UINT64_C (0xFFFFFFFFFFFFFFFF)});
    EXPECT_EQ ("ffffffffffffffff", str.str ());
}
