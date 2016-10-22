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

#include "comdat_scanner.hpp"

#include <gmock/gmock.h>

using output_vector = comdat_scanner::output_vector;
using comdat_map = comdat_scanner::comdat_map;
using sizes = comdat_scanner::sizes;

TEST (ComdatScannerOutput, Eq) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 1, 2, 3 };
    EXPECT_EQ (a, b);
}

TEST (ComdatScannerOutput, Ne1) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 2, 2, 3 };
    EXPECT_NE (a, b);
}
TEST (ComdatScannerOutput, Ne2) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 1, 1, 3 };
    EXPECT_NE (a, b);
}
TEST (ComdatScannerOutput, Ne3) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 1, 2, 1 };
    EXPECT_NE (a, b);
}
TEST (ComdatScannerOutput, Lt1) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 2, 2, 3 };
    EXPECT_LT (a, b);
}
TEST (ComdatScannerOutput, Lt2) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 1, 3, 3 };
    EXPECT_LT (a, b);
}
TEST (ComdatScannerOutput, Lt3) {
    comdat_scanner::output a { 1, 2, 3 };
    comdat_scanner::output b { 1, 2, 4 };
    EXPECT_LT (a, b);
}


TEST (ComdatScannerBuildOutputVector, Empty) {
    using ::testing::ContainerEq;

    comdat_map cm {};
    output_vector const expected {
    };
    
    output_vector const actual = comdat_scanner::build_output_vector (cm);
    EXPECT_THAT (actual, ContainerEq (expected));
}

TEST (ComdatScannerBuildOutputVector, TwoComdatsEachWithMultipleSameSizeInstances) {
    using ::testing::ContainerEq;

    comdat_map cm {
        {
            "foo", // symbol name
            {
                2 * 3, // total size: two instances of 3 bytes each
                3, // largest: the largest instance was 3 bytes.
                2, // number of instances
            },
        },
        {
            "bar", // symbol name
            {
                5 * 7, // total size
                5, // largest
                7, // number of instances
            },
        },
    };
    output_vector const expected {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 3 },
        { /*largest:*/ 5, /*instances:*/ 7, /*wasted:*/ 5 * 6 },
    };

    output_vector const actual = comdat_scanner::build_output_vector (cm);
    EXPECT_THAT (actual, ContainerEq (expected));
}

TEST (ComdatScannerBuildOutputVector, TwoComdatsOneHasOneInstance) {
    using ::testing::ContainerEq;

    comdat_map cm {
        {
            "foo", // symbol name
            {
                4, // total size: instances where 1 & 3 bytes.
                3, // largest: the largest instance was 3 bytes.
                2, // number of instances
            },
        },
        {
            "bar", // symbol name
            {
                5, // total size
                5, // largest: the only instance was 5 bytes.
                1, // number of instances
            },
        },
    };
    output_vector const expected {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 1 },
    };

    output_vector const actual = comdat_scanner::build_output_vector (cm);
    EXPECT_THAT (actual, ContainerEq (expected));
}


TEST (ComdatScannerBuildOutputVector, TwoComdatsOfSameSize) {
    using ::testing::ContainerEq;

    comdat_map cm {
        {
            "bar", // symbol name
            {
                5, // total size
                3, // largest: the only instance was 5 bytes.
                2, // number of instances
            },
        },
        {
            "foo", // symbol name
            {
                4, // total size: instances where 1 & 3 bytes.
                3, // largest: the largest instance was 3 bytes.
                2, // number of instances
            },
        },
    };
    output_vector const expected {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 1 },
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 2 },
    };

    output_vector const actual = comdat_scanner::build_output_vector (cm);
    EXPECT_THAT (actual, ContainerEq (expected));
}


TEST (ComdatScannerTotalComdatSize, Empty) {
    sizes const expected = {
        0, // total size
        0, // total waste
    };
    sizes const actual = comdat_scanner::total_comdat_size (comdat_map {});
    EXPECT_EQ (actual, expected);
}

TEST (ComdatScannerTotalComdatSize, TwoComdats) {
    comdat_map cm {
        {
            "bar", // symbol name
            {
                5, // total size
                3, // largest: the only instance was 5 bytes.
                2, // number of instances
            },
        },
        {
            "foo", // symbol name
            {
                4, // total size: instances where 1 & 3 bytes.
                3, // largest: the largest instance was 3 bytes.
                2, // number of instances
            },
        },
    };

    sizes const expected = {
        5 + 4, // total size
        (5 - 3) + (4 - 3), // total waste
    };
    sizes const actual = comdat_scanner::total_comdat_size (cm);

    EXPECT_EQ (actual, expected);
}


TEST (ComdatScannerFilter, Empty) {
    using ::testing::ContainerEq;
    output_vector in {};
    output_vector const actual = comdat_scanner::filter (in);
    output_vector const expected {};
    EXPECT_THAT (actual, ContainerEq (expected));
}

TEST (ComdatScannerFilter, TwoEntriesNoneFiltered) {
    using ::testing::ContainerEq;
    output_vector in {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 3 },
        { /*largest:*/ 5, /*instances:*/ 7, /*wasted:*/ 5 * 6 },
    };
    output_vector const actual = comdat_scanner::filter (in);
    output_vector const expected {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 3 },
        { /*largest:*/ 5, /*instances:*/ 7, /*wasted:*/ 5 * 6 },
    };
    EXPECT_THAT (actual, ContainerEq (expected));
}

TEST (ComdatScannerFilter, TwoEntriesInSamePosition) {
    using ::testing::ContainerEq;
    output_vector in {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 3 },
        { /*largest:*/ 5, /*instances:*/ 7, /*wasted:*/ 5 * 6 },
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 4 },
    };
    output_vector const actual = comdat_scanner::filter (in);
    output_vector const expected {
        { /*largest:*/ 3, /*instances:*/ 2, /*wasted:*/ 4 }, // The larger of the two "wasted" values.
        { /*largest:*/ 5, /*instances:*/ 7, /*wasted:*/ 5 * 6 },
    };
    EXPECT_THAT (actual, ContainerEq (expected));
}

// eof unittes/test_comdat_scanner.cpp
