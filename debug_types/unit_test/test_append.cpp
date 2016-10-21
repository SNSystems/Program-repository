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

#include "append.hpp"

#include <array>
#include <vector>

#include <gmock/gmock.h>

TEST (Append, String) {
    std::vector<char> s;
    append_string (std::string{"Hello"}, std::back_inserter (s));
    EXPECT_THAT (s, ::testing::ElementsAre ('H', 'e', 'l', 'l', 'o', '\0'));
}

TEST (Append, AttributeContainer) {
    std::vector<char> s;
    std::array<char, 5> value{{'\x04', '\x05', '\x06', '\x07', '\x08'}};
    append (2, 3, value, std::back_inserter (s));
    EXPECT_THAT (
        s, ::testing::ElementsAre ('A', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08'));
}

TEST (Append, AttributeString) {
    std::vector<char> s;
    append (2, 3, std::string{"Hello"}, std::back_inserter (s));
    EXPECT_THAT (s, ::testing::ElementsAre ('A', '\x02', '\x03', 'H', 'e', 'l', 'l', 'o', '\0'));
}
