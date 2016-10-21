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

#ifndef AS_HEX_HPP
#define AS_HEX_HPP

#include <boost/io/ios_state.hpp>
#include <iomanip>
#include <iosfwd>
#include <type_traits>

template <typename T>
struct hex_proxy {
    static_assert (std::is_unsigned<T>::value, "hex_proxy type must be unsigned");
    hex_proxy (T const & v, unsigned d)
            : value (v)
            , digits (d) {}
    T const value;
    unsigned const digits;
};

template <typename T>
std::ostream & operator<< (std::ostream & os, hex_proxy<T> const & h) {
    boost::io::ios_flags_saver ifs (os);
    return os << std::setw (h.digits) << std::setfill ('0') << std::right << std::hex << h.value;
}

template <typename T>
inline hex_proxy<T> as_hex (T const & t) {
    return hex_proxy<T> (t, sizeof (T) * 2);
}

inline hex_proxy<unsigned> as_hex (std::uint8_t t) {
    return {t, 2U};
}

#endif // AS_HEX_HPP
// eof as_hex.hpp
