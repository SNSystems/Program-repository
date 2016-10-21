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

#include "mock_debug.hpp"

// *********************
// * Dwarf_Attribute_s *
// *********************
// (ctor)
// ~~~~~~
Dwarf_Attribute_s::Dwarf_Attribute_s (Dwarf_Half tag_, Dwarf_Signed s_)
        : tag (tag_)
        , form (DW_FORM_sdata)
        , s (s_) {}

Dwarf_Attribute_s::Dwarf_Attribute_s (Dwarf_Half tag_, std::string const & str_)
        : tag (tag_)
        , form (DW_FORM_string)
        , str (str_) {}

Dwarf_Attribute_s::Dwarf_Attribute_s (Dwarf_Half tag_, Dwarf_Die_s const * ref_)
        : tag (tag_)
        , form (DW_FORM_ref_udata)
        , ref (ref_) {}


// ***************
// * Dwarf_Die_s *
// ***************
// (ctor)
// ~~~~~~
Dwarf_Die_s::Dwarf_Die_s () {}

Dwarf_Die_s::Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag)
        : offset_ (offset)
        , tag_ (tag) {}

Dwarf_Die_s::Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag, Dwarf_Die_s const * sibling,
                          Dwarf_Die_s const * child)
        : offset_ (offset)
        , tag_ (tag)
        , sibling_ (sibling)
        , child_ (child) {}

Dwarf_Die_s::Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag,
                          std::initializer_list<Dwarf_Attribute_s> l)
        : offset_ (offset)
        , tag_ (tag) {
    this->build_attr_list (l);
}

Dwarf_Die_s::Dwarf_Die_s (Dwarf_Off offset, Dwarf_Half tag, Dwarf_Die_s const * sibling,
                          Dwarf_Die_s const * child, std::initializer_list<Dwarf_Attribute_s> l)
        : offset_ (offset)
        , tag_ (tag)
        , sibling_ (sibling)
        , child_ (child) {
    this->build_attr_list (l);
}

Dwarf_Die_s::Dwarf_Die_s (Dwarf_Half tag)
        : tag_ (tag) {}
Dwarf_Die_s::Dwarf_Die_s (Dwarf_Half tag, std::initializer_list<Dwarf_Attribute_s> l)
        : tag_ (tag) {
    this->build_attr_list (l);
}

Dwarf_Die_s::Dwarf_Die_s (Dwarf_Die_s const & rhs)
        : offset_ (rhs.offset_)
        , tag_ (rhs.tag_)
        , sibling_ (rhs.sibling_)
        , child_ (rhs.child_)
        , attributes_ (rhs.attributes_) {

    this->build_attr_ptrs ();
}

// operator=
// ~~~~~~~~~
Dwarf_Die_s & Dwarf_Die_s::operator= (Dwarf_Die_s const & rhs) {
    if (this != &rhs) {
        offset_ = rhs.offset_;
        tag_ = rhs.tag_;
        sibling_ = rhs.sibling_;
        child_ = rhs.child_;
        attributes_ = rhs.attributes_;
        this->build_attr_ptrs ();
    }
    return *this;
}

// build_attr_list
// ~~~~~~~~~~~~~~~
void Dwarf_Die_s::build_attr_list (std::initializer_list<Dwarf_Attribute_s> l) {
    attributes_.clear ();
    attributes_.reserve (l.size ());
    std::copy (std::begin (l), std::end (l), std::back_inserter (attributes_));
    this->build_attr_ptrs ();
}

// build_attr_ptrs
// ~~~~~~~~~~~~~~~
void Dwarf_Die_s::build_attr_ptrs () {
    attribute_ptrs_.clear ();
    attribute_ptrs_.reserve (attributes_.size ());
    for (Dwarf_Attribute_s & att : attributes_) {
        attribute_ptrs_.push_back (&att);
    }
}
// eof mock_debug.cpp
