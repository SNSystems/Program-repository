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

#ifndef APPEND_HPP
#define APPEND_HPP

#include <algorithm>
#include <iterator>
#include <string>
#include <tuple>

#include "build_contexts.hpp"
#include "dwarf_helpers.hpp"
#include "leb128.hpp"
#include <dwarf.h>
#include <libdwarf.h>

template <typename SequenceOutputIterator>
SequenceOutputIterator append_string (std::string const & str, SequenceOutputIterator S) {
    S = std::copy (std::begin (str), std::end (str), S);
    *(S++) = '\0';
    return S;
}

template <typename ValueType, typename SequenceOutputIterator>
SequenceOutputIterator append (Dwarf_Half const tag, Dwarf_Half const form, ValueType const & value,
                               SequenceOutputIterator S) {
    *(S++) = 'A';
    S = encode_uleb128 (tag, S);
    S = encode_uleb128 (form, S);
    S = std::copy (std::begin (value), std::end (value), S);
    return S;
}

template <typename SequenceOutputIterator>
SequenceOutputIterator append (Dwarf_Half const tag, Dwarf_Half const form, std::string const & str,
                               SequenceOutputIterator S) {
    *(S++) = 'A';
    S = encode_uleb128 (tag, S);
    S = encode_uleb128 (form, S);
    S = append_string (str, S);
    return S;
}



template <typename SequenceIterator>
SequenceIterator append_type_die_context (dwarf::debug * const debug, Dwarf_Die die,
                                          SequenceIterator S, die_context_map const & contexts) {
    // The contexts container holds a map of DIE offset to its corresponding
    // context string, so we can simply append that here.
    auto it = contexts.find (debug->die_to_offset (die));
    std::string const & s = (it != std::end (contexts)) ? std::get<0> (it->second) : std::string ();
    return std::copy (std::begin (s), std::end (s), S);
}


class block_container {
public:
    explicit block_container (dwarf::attribute::block_ptr && block)
            : block_ (std::move (block)) {}
    std::uint8_t const * begin () const {
        return reinterpret_cast<std::uint8_t const *> (block_->bl_data);
    }
    std::uint8_t const * end () const {
        return this->begin () + block_->bl_len;
    }

private:
    dwarf::attribute::block_ptr block_;
};


template <typename SequenceOutputIterator>
SequenceOutputIterator append_attribute (dwarf::attribute const & attribute,
                                         SequenceOutputIterator S) {
    Dwarf_Half const tag = attribute.what_attr ();
    Dwarf_Half const form = attribute.what_form ();
    switch (form) {
    case DW_FORM_flag:
    case DW_FORM_flag_present:
        *(S++) = 'A';
        S = encode_uleb128 (tag, S);
        S = encode_uleb128 (DW_FORM_flag, S);
        *(S++) = (form == DW_FORM_flag_present || attribute.flag ()) ? 1 : 0;
        break;

    // string class
    case DW_FORM_string:
    case DW_FORM_strp:
        // case DW_FORM_GNU_str_index: ... do these belong here?
        // case DW_FORM_strx
        S = append (tag, DW_FORM_string, attribute.string (), S);
        break;

    // block class
    case DW_FORM_block:
    case DW_FORM_block1:
    case DW_FORM_block2:
    case DW_FORM_block4:
        S = append (tag, DW_FORM_block, block_container{attribute.block ()}, S);
        break;

    // constant class
    case DW_FORM_sdata:
        *(S++) = 'A';
        S = encode_uleb128 (tag, S);
        S = encode_uleb128 (DW_FORM_sdata, S);
        S = encode_sleb128 (attribute.signed_constant (), S);
        break;

    case DW_FORM_data1:
    case DW_FORM_data2:
    case DW_FORM_data4:
    case DW_FORM_data8:
    case DW_FORM_udata:
        *(S++) = 'A';
        S = encode_uleb128 (tag, S);
        S = encode_uleb128 (DW_FORM_sdata, S);
        // FIXME: the return type of unsigned_constant is uint64 and the argument to
        // encode_sleb128 is an int64: there's a possibility of truncation.
        S = encode_sleb128 (attribute.unsigned_constant (), S);
        break;

    default:
        break;
    }
    return S;
}

#endif // APPEND_HPP
