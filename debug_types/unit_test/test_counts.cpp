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

#include "counts.hpp"
#include <gtest/gtest.h>

TEST (Counts, InitialState) {
    counts c (97U);
    EXPECT_EQ (97U, c.total ());
    EXPECT_EQ (0U, c.types ());
    EXPECT_EQ (0U, c.unique ());
    EXPECT_EQ ("Unknown", c.producer ());
}

TEST (Counts, OneType) {
    counts c (97U);
    auto const signature = std::uint64_t{65537};
    c.add_type (signature, "producer");
    EXPECT_EQ (97U, c.total ());
    EXPECT_EQ (1U, c.types ());
    EXPECT_EQ (1U, c.unique ());
    EXPECT_EQ ("producer", c.producer ());
}

TEST (Counts, TwoIdenticalTypes) {
    counts c (2U);
    auto const signature = std::uint64_t{65537};
    c.add_type (signature, "producer");
    c.add_type (signature, "producer");
    EXPECT_EQ (2U, c.total ());
    EXPECT_EQ (2U, c.types ());
    EXPECT_EQ (1U, c.unique ());
    EXPECT_EQ ("producer", c.producer ());
}

TEST (Counts, TwoDifferentTypesSameProducer) {
    counts c (2U);
    c.add_type (std::uint64_t{65537}, "producer1");
    c.add_type (std::uint64_t{97871}, "producer2");
    EXPECT_EQ (2U, c.total ());
    EXPECT_EQ (2U, c.types ());
    EXPECT_EQ (2U, c.unique ());
    EXPECT_EQ ("producer1/producer2", c.producer ());
}

TEST (Counts, OneTypeComplexProducer) {
    counts c (1U);
    auto const signature = std::uint64_t{65537};
    c.add_type (signature, "GNU C 4.8.4 -mtune=generic -march=x86-64");
    EXPECT_EQ (1U, c.total ());
    EXPECT_EQ (1U, c.types ());
    EXPECT_EQ (1U, c.unique ());
    EXPECT_EQ ("GNU C 4.8.4", c.producer ());
}

TEST (Counts, OneTypeProducerWithVersion) {
    counts c (2U);
    auto const signature = std::uint64_t{65537};
    char const * producer = "clang version 3.9.0 (trunk 269902)";
    c.add_type (signature, producer);
    c.add_type (signature, producer);
    EXPECT_EQ (2U, c.total ());
    EXPECT_EQ (2U, c.types ());
    EXPECT_EQ (1U, c.unique ());
    EXPECT_EQ ("clang v 3.9.0 (trunk 269902)", c.producer ());
}

// eof test_counts.cpp
