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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <limits>

namespace md5 {

    // context
    // ~~~~~~~
    context::context ()
            : finalized_ (false) {
        MD5Init (&ctx_);
    }

    // update
    // ~~~~~~
    void context::update (void const * src, std::size_t len) {
        assert (!finalized_);
        split<unsigned int> (src, len, [this](void const * buffer, unsigned int l) {
            MD5Update (&ctx_, static_cast<unsigned char const *> (buffer), l);
        });
    }

    void context::update (char const * str) {
        this->update (str, std::strlen (str));
    }

    // finalize
    // ~~~~~~~~
    digest context::finalize () {
        assert (!finalized_);
        digest d;
        MD5Final (d.data (), &ctx_);
        finalized_ = true;
        return d;
    }

    // digest_hex
    // ~~~~~~~~~~
    std::string context::digest_hex (digest const & d) {
        auto to_hex = [](digest::value_type v) -> char {
            assert (v < 0x10);
            return static_cast<digest::value_type> (v + ((v < 10) ? '0' : 'a' - 10));
        };

        std::string result;
        result.reserve (d.size () * 2);
        for (auto c : d) {
            result.push_back (to_hex ((c >> 4) & 0x0F));
            result.push_back (to_hex (c & 0x0F));
        }
        return result;
    }

    // split
    // ~~~~~
    template <typename WidthType, typename Function>
    void context::split (void const * buffer, std::size_t size, Function const & function) {

        static std::size_t const max = std::numeric_limits<WidthType>::max ();
        while (size > 0) {
            auto const chunk_size = static_cast<WidthType> (std::min (max, size));
            function (buffer, chunk_size);
            buffer = static_cast<std::uint8_t const *> (buffer) + chunk_size;
            assert (chunk_size >= size);
            size -= chunk_size;
        }
    }

} // namespace md5
// eof md5_context.cpp
