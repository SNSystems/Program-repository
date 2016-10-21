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

#include "md5.h"
#include <sstream>

#include <gtest/gtest.h>

TEST (Md5, Empty) {
    md5::hasher h;
    h.append (std::string (""));
    md5::digest const actual = h.finish ();
    md5::digest const expected{{0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04, 0xE9, 0x80, 0x09,
                                0x98, 0xEC, 0xF8, 0x42, 0x7E}};
    EXPECT_EQ (expected, actual);
}


// Let the message be the ASCII string "abc". The resulting 128-bit message digest is 90015098
// 3CD24FB0 D6963F7D 28E17F72.
TEST (Md5, NsrlAbc) {
    md5::hasher h;
    h.append (std::string{"abc"});
    md5::digest const actual = h.finish ();
    md5::digest const expected{{
        0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0, 0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F,
        0x72,
    }};
    EXPECT_EQ (expected, actual);
}

// Let the message be the ASCII string "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq".
// The resulting 128-bit message digest is 8215EF07 96A20BCA AAE116D3 876C664A.
TEST (Md5, NsrlAlphaSequence) {
    md5::hasher h;
    h.append (std::string{"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"});
    md5::digest const actual = h.finish ();
    md5::digest const expected{{
        0x82, 0x15, 0xEF, 0x07, 0x96, 0xA2, 0x0B, 0xCA, 0xAA, 0xE1, 0x16, 0xD3, 0x87, 0x6C, 0x66,
        0x4A,
    }};
    EXPECT_EQ (expected, actual);
}

// Let the message be the binary-coded form of the ASCII string which consists of 1,000,000
// repetitions of
// the character "a". [file] The resulting MD5 message digest is 7707D6AE 4E027C70 EEA2A935
// C2296F21.
TEST (Md5, NsrlMillionAs) {
    md5::hasher h;
    h.append (std::string (1000000, 'a'));
    md5::digest const actual = h.finish ();
    md5::digest const expected{{
        0x77, 0x07, 0xD6, 0xAE, 0x4E, 0x02, 0x7C, 0x70, 0xEE, 0xA2, 0xA9, 0x35, 0xC2, 0x29, 0x6F,
        0x21,
    }};
    EXPECT_EQ (expected, actual);
}

// eof test_md5.cpp
