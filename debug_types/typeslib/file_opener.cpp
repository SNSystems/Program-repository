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

#include "file_opener.hpp"

/* for 'open' */
#if defined(_WIN32)
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <fcntl.h>


file_opener::file_opener (boost::filesystem::path const & file_path)
        : fd_ (open_file (file_path)) {
    if (fd_ == -1) {
        throw std::system_error (errno, std::generic_category ());
    }
}

file_opener::~file_opener () noexcept {
    ::close (fd_);
}


int file_opener::open_file (boost::filesystem::path const & file_path) {
#ifdef _WIN32
    return ::_wopen (file_path.c_str (), O_RDONLY | O_BINARY);
#else
    return ::open (file_path.c_str (), O_RDONLY);
#endif
}
