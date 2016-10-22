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

#include "md5_context.h"
#include "gtest/gtest.h"

TEST (md5, empty) {
    md5::context c;
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "d41d8cd98f00b204e9800998ecf8427e");
}

TEST (md5, a) {
    md5::context c;
    c.update ("a");
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "0cc175b9c0f1b6a831c399e269772661");
}

TEST (md5, abc) {
    md5::context c;
    c.update ("abc");
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "900150983cd24fb0d6963f7d28e17f72");
}

TEST (md5, str1) {
    md5::context c;
    c.update ("message digest");
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "f96b697d7cb7938d525a2f31aaf161d0");
}

TEST (md5, alphabet) {
    md5::context c;
    c.update ("abcdefghijklmnopqrstuvwxyz");
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "c3fcd3d76192e4007dfb496cca67e13b");
}

TEST (md5, alphabet_digits_mixed_case) {
    md5::context c;
    c.update (std::string ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "d174ab98d277d9f5a5611c2c9f419d9f");
}

TEST (md5, digits) {
    md5::context c;
    c.update ("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    std::string const actual = md5::context::digest_hex (c.finalize ());
    EXPECT_EQ (actual, "57edf4a22be3c955ac49da2e2107b67a");
}
// eof test_md5.cpp
