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

#ifndef UNITTEST_TEMPORARY_FILE_H
#define UNITTEST_TEMPORARY_FILE_H (1)

#include <cstdio>
#include <memory>

typedef std::unique_ptr <FILE, decltype (&std::fclose)>  file_ptr;

file_ptr temporary_file ();

/// This function is the largely the opposite of a temporary file, but it's useful when
/// diagnosing a test failure. Rather than creating a temporary file and deleting that file
/// when it is closed by the destructor, an explicit path is used and the file is not deleted.
file_ptr temporary_file (char const * path);

#endif // UNITTEST_TEMPORARY_FILE_H
// eof unittest/temporary_file.h
