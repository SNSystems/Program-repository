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

#ifndef MOCK_DEBUG_HPP
#define MOCK_DEBUG_HPP

#include <stdexcept>
#include <vector>

#include "dwarf_helpers.hpp"
#include <dwarf.h>
#include <gmock/gmock.h>



struct Dwarf_Die_s;

struct Dwarf_Attribute_s {
    Dwarf_Attribute_s (Dwarf_Half tag_, Dwarf_Signed s_);
    Dwarf_Attribute_s (Dwarf_Half tag_, std::string const & str_);
    Dwarf_Attribute_s (Dwarf_Half tag_, Dwarf_Die_s const * ref_);

    Dwarf_Half tag;
    Dwarf_Half form;
    Dwarf_Signed s = 0;
    Dwarf_Die_s const * ref = nullptr;
    std::string str;
};



struct Dwarf_Die_s {
public:
    Dwarf_Die_s ();
    Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag);
    Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag, Dwarf_Die_s const * sibling,
                 Dwarf_Die_s const * child);
    Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag, std::initializer_list<Dwarf_Attribute_s> l);
    Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag, Dwarf_Die_s const * sibling,
                 Dwarf_Die_s const * child, std::initializer_list<Dwarf_Attribute_s> l);

    Dwarf_Die_s (Dwarf_Half tag);
    Dwarf_Die_s (Dwarf_Half tag, std::initializer_list<Dwarf_Attribute_s> l);


    Dwarf_Die_s (Dwarf_Die_s const & rhs);
    Dwarf_Die_s & operator= (Dwarf_Die_s const & rhs);

    // DIE properties.
    Dwarf_Off offset () const {
        return offset_;
    }
    Dwarf_Half tag () const {
        return tag_;
    }

    Dwarf_Die_s const * sibling () const {
        return sibling_;
    }
    Dwarf_Die_s const * child () const {
        return child_;
    }
    Dwarf_Die_s & sibling (Dwarf_Die_s * const s) {
        sibling_ = s;
        return *this;
    }
    Dwarf_Die_s & child (Dwarf_Die_s * const s) {
        child_ = s;
        return *this;
    }

    void offset (Dwarf_Off offset) {
        offset_ = offset;
    }

    template <class... Args>
    void add_attribute (Args &&... args) {
        attributes_.emplace_back (std::forward<Args> (args)...);
        this->build_attr_ptrs ();
    }

    // Attribute access.
    using attribute_container = std::vector<Dwarf_Attribute_s>;
    attribute_container::const_iterator begin () const {
        return attributes_.begin ();
    }
    attribute_container::const_iterator end () const {
        return attributes_.end ();
    }
    std::size_t size () const {
        return attributes_.size ();
    }
    bool empty () const {
        return attributes_.empty ();
    }
    Dwarf_Attribute * attrlist () {
        return &attribute_ptrs_[0];
    }

private:
    Dwarf_Off offset_ = 0;
    Dwarf_Half tag_ = 0;
    Dwarf_Die_s const * sibling_ = nullptr;
    Dwarf_Die_s const * child_ = nullptr;

    attribute_container attributes_;
    std::vector<Dwarf_Attribute> attribute_ptrs_;

    void build_attr_list (std::initializer_list<Dwarf_Attribute_s> l);
    void build_attr_ptrs ();
};


class mock_debug : public dwarf::debug {
public:
    mock_debug ()
            : dwarf::debug (nullptr) {}

    MOCK_METHOD5 (next_cu_header,
                  bool(Dwarf_Unsigned * cu_header_length /*out*/,
                       Dwarf_Half * version_stamp /*out*/, Dwarf_Off * abbrev_offset /*out*/,
                       Dwarf_Half * address_size /*out*/,
                       Dwarf_Unsigned * next_cu_header_offset /*out*/));

    MOCK_METHOD1 (die_to_offset, Dwarf_Off (Dwarf_Die die));

    MOCK_METHOD1 (get_child, Dwarf_Die (Dwarf_Die)); // a mockable wrapper for child().
    dwarf::die_ptr child (Dwarf_Die die) final {
        return this->as_die_ptr (this->get_child (die));
    }

    MOCK_METHOD1 (get_siblingof, Dwarf_Die (Dwarf_Die)); // a mockable wrapper for siblingof().
    dwarf::die_ptr siblingof (Dwarf_Die debug) final {
        return this->as_die_ptr (this->get_siblingof (debug));
    }

    MOCK_METHOD1 (get_offset_to_die,
                  Dwarf_Die (Dwarf_Off offset)); // a mockable wrapper for offset_to_die()
    dwarf::die_ptr offset_to_die (Dwarf_Off offset) final {
        return this->as_die_ptr (this->get_offset_to_die (offset));
    }

    // DIE properties.
    MOCK_METHOD1 (tag, Dwarf_Half (Dwarf_Die die));
    MOCK_METHOD1 (name, std::string (Dwarf_Die die));
    MOCK_METHOD2 (attrlist, Dwarf_Signed (Dwarf_Die die, Dwarf_Attribute ** array));

    // Attributes
    MOCK_METHOD2 (attribute_from_tag, dwarf::owned_attribute (Dwarf_Die die, Dwarf_Half tag));
    MOCK_CONST_METHOD1 (attribute_what_attr, Dwarf_Half (Dwarf_Attribute a));
    MOCK_CONST_METHOD1 (attribute_what_form, Dwarf_Half (Dwarf_Attribute a));

    // Attribute forms
    MOCK_CONST_METHOD1 (attribute_form_string, std::string (Dwarf_Attribute a));
    MOCK_CONST_METHOD1 (attribute_form_flag, bool(Dwarf_Attribute a));
    MOCK_CONST_METHOD1 (attribute_form_signed_constant, Dwarf_Signed (Dwarf_Attribute a));
    MOCK_CONST_METHOD1 (attribute_form_unsigned_constant, Dwarf_Unsigned (Dwarf_Attribute a));
    MOCK_METHOD1 (attribute_form_ref, Dwarf_Off (Dwarf_Attribute a));

    MOCK_METHOD1 (
        get_attribute_form_block,
        Dwarf_Block *(Dwarf_Attribute a)); // a mockable wrapper for attribute_form_block()
    dwarf::attribute::block_ptr attribute_form_block (Dwarf_Attribute a) {
        Dwarf_Block * block = this->get_attribute_form_block (a);
        return {block, [](Dwarf_Block * bl) { delete bl; }};
    }


    MOCK_METHOD2 (dealloc, void(Dwarf_Ptr space, Dwarf_Unsigned alloc_type));

    static dwarf::die_ptr new_die () {
        return {new Dwarf_Die_s, [](Dwarf_Die d) { delete d; }};
    }
};

#endif // MOCK_DEBUG_HPP
// eof mock_debug.hpp
