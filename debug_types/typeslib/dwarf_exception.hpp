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

#ifndef DWARF_EXCEPTION_H
#define DWARF_EXCEPTION_H

#include <exception>
#include <iosfwd>
#include <libdwarf.h>

class dwarf_exception : public std::exception {
public:
    dwarf_exception (Dwarf_Debug debug, char const * function_called, Dwarf_Error error);
    ~dwarf_exception () noexcept final;

    dwarf_exception (dwarf_exception && rhs);
    dwarf_exception & operator= (dwarf_exception && rhs);

    virtual char const * what () const noexcept final;

    std::ostream & write (std::ostream & os) const;

private:
    Dwarf_Debug debug_;
    char const * function_called_;
    Dwarf_Error error_;
};

inline std::ostream & operator<< (std::ostream & os, dwarf_exception const & ex) {
    return ex.write (os);
}

#endif // DWARF_EXCEPTION_H

// eof dwarf_exception.h
