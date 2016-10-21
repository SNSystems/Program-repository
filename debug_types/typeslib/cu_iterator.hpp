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

#ifndef CU_ITERATOR_HPP
#define CU_ITERATOR_HPP

#include "dwarf_helpers.hpp"
#include <boost/iterator/iterator_facade.hpp>
#include <libdwarf.h>

/// \brief Provides a forward iterator interface for CUs.
///
/// cu_iterator is a class that provides an interface that meets the
/// requirements of a standard library forward iterator. The hard work of that
/// is done by the Boost iterator_facade, and the implementation here simply
/// supplies the methods that that requires.

class cu_iterator
    : public boost::iterator_facade<cu_iterator, dwarf::die_ptr, boost::forward_traversal_tag> {
public:
    cu_iterator (dwarf::debug * debug, Dwarf_Die die);
    cu_iterator (dwarf::debug * debug);

private:
    friend class boost::iterator_core_access;

    /// Navigates to the next CU.
    void increment () {
        die_ = next_cu_.next (debug_);
    }

    bool equal (cu_iterator const & other) const {
        return die_ == other.die_;
    }

    dwarf::die_ptr & dereference () const {
        return die_;
    }

    struct next_cu_info {
        dwarf::die_ptr next (dwarf::debug * debug);

        Dwarf_Unsigned cu_header_length = 0;
        Dwarf_Unsigned abbrev_offset = 0;
        Dwarf_Half version_stamp = 0;
        Dwarf_Half address_size = 0;
        Dwarf_Unsigned next_cu_offset = 0;
    };

    dwarf::debug * debug_;
    next_cu_info next_cu_;
    mutable dwarf::die_ptr die_;
};


class cu_container {
public:
    explicit cu_container (dwarf::debug * debug)
            : debug_ (debug) {}

    dwarf::debug * debug () const {
        return debug_;
    }

    cu_iterator begin () {
        return {debug_};
    }
    cu_iterator end () {
        return {debug_, nullptr};
    }

private:
    dwarf::debug * debug_;
};


#endif // CU_ITERATOR_HPP

// eof cu_iterator.hpp
