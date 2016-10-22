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

#ifndef MD5_CONTEXT_H
#define MD5_CONTEXT_H  (1)

#include <array>
#include <string>
#include "md5.h"

namespace md5 {

    typedef std::array <unsigned char, 16>  digest;

    class context {
    public:
        context ();

        context (context const & ) = delete;
        context & operator= (context const & ) = delete;

        void update (std::string const & str) {
            this->update (str.data (), str.length ());
        }
        void update (char const * str);
        void update (void const * src, std::size_t len);
        digest finalize ();
        static std::string digest_hex (digest const & d);

    private:
        template <typename WidthType, typename Function>
        void split (void const * buffer, std::size_t size, Function const & function);

        MD5_CTX ctx_;
        bool finalized_;
    };

} // namespace md5

#endif // MD5_CONTEXT_H
// eof md5_context.h
