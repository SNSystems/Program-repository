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

#ifndef SORT_ATTRIBUTES_HPP
#define SORT_ATTRIBUTES_HPP

#include "dwarf_helpers.hpp"
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>

namespace details {
    using attribute_order = std::vector<unsigned>;
    constexpr auto default_priority = std::numeric_limits<attribute_order::value_type>::max ();
    extern attribute_order const order;
    extern std::size_t const order_size;
}

template <typename AttributeIterator>
AttributeIterator sort_attributes (AttributeIterator first, AttributeIterator last) {
    using namespace details;

    // First sort the attributes into priority order (that is, the order specified by the
    // DWARF spec. for the order in which they should be added to the hash).
    std::sort (first, last, [](dwarf::attribute const & lhs, dwarf::attribute const & rhs) {
        auto const lhs_tag = lhs.what_attr ();
        auto const rhs_tag = rhs.what_attr ();
        auto const lhs_priority = lhs_tag >= order_size ? default_priority : order[lhs_tag];
        auto const rhs_priority = rhs_tag >= order_size ? default_priority : order[rhs_tag];
        return lhs_priority < rhs_priority;
    });

    // Now find the first attribute not explicitly given in the specification.
    return std::find_if (first, last, [](dwarf::attribute const & attribute) {
        auto const tag = attribute.what_attr ();
        return tag >= order_size || order[tag] == default_priority;
    });
}

#endif // SORT_ATTRIBUTES_HPP
// eof sort_attributes.hpp
