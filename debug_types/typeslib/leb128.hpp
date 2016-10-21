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

#ifndef LEB128_HPP
#define LEB128_HPP

#include <cstdint>

/// Utility function to encode a SLEB128 value to an output stream.
template <typename OutputIterator>
OutputIterator encode_sleb128 (std::int64_t value, OutputIterator output) {
    bool is_more;
    do {
        std::uint8_t byte = value & 0x7f;
        // NOTE: this assumes that this signed shift is an arithmetic right shift.
        value >>= 7;
        is_more = !((value == 0 && (byte & 0x40) == 0) || (value == -1 && (byte & 0x40) != 0));
        if (is_more) {
            byte |= 0x80; // Mark this byte to show that more bytes will follow.
        }

        *(output++) = byte;
    } while (is_more);
    return output;
}


template <typename OutputIterator>
OutputIterator encode_uleb128 (std::uint64_t value, OutputIterator output) {
    do {
        std::uint8_t byte = value & 0x7f;
        value >>= 7;
        if (value != 0) {
            byte |= 0x80; // Mark this byte to show that more bytes will follow.
        }
        *(output++) = byte;
    } while (value != 0);
    return output;
}

#endif // LEB128_HPP
// eof leb128.hpp
