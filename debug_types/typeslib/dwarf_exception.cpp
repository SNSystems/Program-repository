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

#include "dwarf_exception.hpp"
#include <iostream>
#include <stdexcept>
#include <utility>

dwarf_exception::dwarf_exception (Dwarf_Debug debug, char const * function_called,
                                  Dwarf_Error error)
        : debug_ (debug)
        , function_called_ (function_called)
        , error_ (error) {}

dwarf_exception::dwarf_exception (dwarf_exception && rhs)
        : debug_ (std::move (rhs.debug_))
        , function_called_ (std::move (rhs.function_called_))
        , error_ (std::move (rhs.error_)) {
    rhs.debug_ = nullptr;
    rhs.function_called_ = nullptr;
    rhs.error_ = nullptr;
}

dwarf_exception::~dwarf_exception () noexcept {
    ::dwarf_dealloc (debug_, error_, DW_DLA_ERROR);
}

dwarf_exception & dwarf_exception::operator= (dwarf_exception && rhs) {
    if (&rhs != this) {
        ::dwarf_dealloc (debug_, error_, DW_DLA_ERROR);
        error_ = std::move (rhs.error_);
        rhs.error_ = nullptr;

        function_called_ = std::move (rhs.function_called_);
        rhs.function_called_ = nullptr;

        debug_ = std::move (rhs.debug_);
        rhs.debug_ = nullptr;
    }
    return *this;
}


char const * dwarf_exception::what () const noexcept {
    return "DWARF error";
}

std::ostream & dwarf_exception::write (std::ostream & os) const {
    return os << function_called_ << ": " << ::dwarf_errmsg (error_) << " ("
              << ::dwarf_errno (error_) << ")";
}

// eof print_error.cpp
