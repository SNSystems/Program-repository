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

#include "dwarf_helpers.hpp"

#include <cassert>
#include <dwarf.h>
#include <limits>

#include "dwarf_exception.hpp"

namespace dwarf {
    bool debug::next_cu_header (Dwarf_Unsigned * cu_header_length, Dwarf_Half * version_stamp,
                                Dwarf_Off * abbrev_offset, Dwarf_Half * address_size,
                                Dwarf_Unsigned * next_cu_header_offset) {

        auto error = Dwarf_Error{nullptr};
        switch (::dwarf_next_cu_header (debug_, cu_header_length, version_stamp, abbrev_offset,
                                        address_size, next_cu_header_offset, &error)) {
        case DW_DLV_OK:
            return true;
        case DW_DLV_ERROR:
            throw dwarf_exception (debug_, "dwarf_next_cu_header", error);
        default:
            assert (false);
        case DW_DLV_NO_ENTRY:
            return false;
        }
    }

    // die_to_offset
    // ~~~~~~~~~~~~~
    Dwarf_Off debug::die_to_offset (Dwarf_Die die) {
        auto offset = Dwarf_Off{0};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_dieoffset (die, &offset, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_dieoffset", error);
        }
        return offset;
    }

    // offset_to_die
    // ~~~~~~~~~~~~~
    die_ptr debug::offset_to_die (Dwarf_Off offset) {
        auto error = Dwarf_Error{nullptr};
        auto die = Dwarf_Die{nullptr};
        if (::dwarf_offdie (debug_, offset, &die, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_offdie", error);
        }
        return this->as_die_ptr (die);
    }

    // siblingof
    // ~~~~~~~~~
    die_ptr debug::siblingof (Dwarf_Die die) {
        auto error = Dwarf_Error{nullptr};
        auto caller_ret_die = Dwarf_Die{nullptr};
        switch (::dwarf_siblingof (debug_, die, &caller_ret_die, &error)) {
        case DW_DLV_OK:
            return this->as_die_ptr (caller_ret_die);
        case DW_DLV_ERROR:
            throw dwarf_exception (debug_, "dwarf_siblingof", error);
        default:
            assert (false);
        case DW_DLV_NO_ENTRY:
            return this->as_die_ptr (nullptr);
        }
    }

    // child
    // ~~~~~
    die_ptr debug::child (Dwarf_Die die) {
        auto child = Dwarf_Die{nullptr};
        auto error = Dwarf_Error{nullptr};
        switch (::dwarf_child (die, &child, &error)) {
        case DW_DLV_ERROR:
            throw dwarf_exception (debug_, "dwarf_child", error);
        case DW_DLV_OK:
            return this->as_die_ptr (child);
        default:
            assert (0);
        case DW_DLV_NO_ENTRY:
            return this->as_die_ptr (nullptr);
        }
    }

    // name
    // ~~~~
    std::string debug::name (Dwarf_Die die) {
        auto error = Dwarf_Error{nullptr};
        char * name{nullptr};
        switch (::dwarf_diename (die, &name, &error)) {
        case DW_DLV_ERROR:
            throw dwarf_exception (debug_, "dwarf_diename", error);
        case DW_DLV_OK:
            assert (name != nullptr);
            return {name};
        default:
            assert (0);
        // fallthough
        case DW_DLV_NO_ENTRY:
            return {};
        }
    }

    // tag
    // ~~~
    Dwarf_Half debug::tag (Dwarf_Die die) {
        auto tag = Dwarf_Half{0};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_tag (die, &tag, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_tag", error);
        }
        return tag;
    }

    // attrlist
    // ~~~~~~~~
    Dwarf_Signed debug::attrlist (Dwarf_Die die, Dwarf_Attribute ** array) {
        auto error = Dwarf_Error{nullptr};
        auto size = Dwarf_Signed{0};
        int res = ::dwarf_attrlist (die, array, &size, &error);
        if (res == DW_DLV_ERROR) {
            throw dwarf_exception (debug_, "dwarf_attrlist", error);
        }
        assert (res == DW_DLV_OK || res == DW_DLV_NO_ENTRY);
        assert (size >= 0);
        return size;
    }

    Dwarf_Half debug::attribute_what_attr (Dwarf_Attribute a) const {
        auto attr = Dwarf_Half{0};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_whatattr (a, &attr, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_whatattr", error);
        }
        return attr;
    }
    Dwarf_Half debug::attribute_what_form (Dwarf_Attribute a) const {
        auto form = Dwarf_Half{0};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_whatform (a, &form, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_whatform", error);
        }
        return form;
    }

    std::string debug::attribute_form_string (Dwarf_Attribute a) const {
        char * str{nullptr};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_formstring (a, &str, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_formstring", error);
        }
        // Do not call ever be tempted to call dwarf_dealloc() on the string
        // returned by dwarf_formstring().
        return std::string (str);
    }

    bool debug::attribute_form_flag (Dwarf_Attribute a) const {
        auto error = Dwarf_Error{nullptr};
        auto flag = Dwarf_Bool{false};
        if (::dwarf_formflag (a, &flag, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_formflag", error);
        }
        return flag != 0;
    }

    Dwarf_Signed debug::attribute_form_signed_constant (Dwarf_Attribute a) const {
        auto value = Dwarf_Signed{0};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_formsdata (a, &value, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_formsdata", error);
        }
        return value;
    }

    Dwarf_Unsigned debug::attribute_form_unsigned_constant (Dwarf_Attribute a) const {
        auto value = Dwarf_Unsigned{0};
        auto error = Dwarf_Error{nullptr};
        if (::dwarf_formudata (a, &value, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_formudata", error);
        }
        return value;
    }

    attribute::block_ptr debug::attribute_form_block (Dwarf_Attribute a) {
        auto error = Dwarf_Error{nullptr};
        Dwarf_Block * block{nullptr};
        if (::dwarf_formblock (a, &block, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_formblock", error);
        }

        auto dealloc = [this](Dwarf_Block * a) { this->dealloc (a, DW_DLA_BLOCK); };
        return {block, dealloc};
    }

    Dwarf_Off debug::attribute_form_ref (Dwarf_Attribute a) {
        auto error = Dwarf_Error{nullptr};
        auto offset = Dwarf_Off{0};
        if (::dwarf_global_formref (a, &offset, &error) != DW_DLV_OK) {
            throw dwarf_exception (debug_, "dwarf_global_formref", error);
        }
        return offset;
    }

    // attribute_from_tag
    // ~~~~~~~~~~~~~~~~~~
    dwarf::owned_attribute debug::attribute_from_tag (Dwarf_Die die, Dwarf_Half tag) {
        auto error = Dwarf_Error{nullptr};
        auto attribute = Dwarf_Attribute{nullptr};
        switch (::dwarf_attr (die, tag, &attribute, &error)) {
        case DW_DLV_ERROR:
            throw dwarf_exception (debug_, "dwarf_attr", error);
        case DW_DLV_OK:
            return {this, attribute};
        default:
            assert (0);
        // fallthrough
        case DW_DLV_NO_ENTRY:
            return {this, nullptr};
        }
    }

    // ********************
    // * debug::attribute *
    // ********************
    // what_attr
    // ~~~~~~~~~
    Dwarf_Half attribute::what_attr () const {
        return debug_->attribute_what_attr (att_);
    }

    // what_form
    // ~~~~~~~~~
    Dwarf_Half attribute::what_form () const {
        return debug_->attribute_what_form (att_);
    }

    auto attribute::block () const -> attribute::block_ptr {
        return debug_->attribute_form_block (att_);
    }


    Dwarf_Off attribute::ref () const {
        return debug_->attribute_form_ref (att_);
    }

    bool attribute::flag () const {
        return debug_->attribute_form_flag (att_);
    }

    std::string attribute::string () const {
        return debug_->attribute_form_string (att_);
    }

    Dwarf_Signed attribute::signed_constant () const {
        return debug_->attribute_form_signed_constant (att_);
    }

    Dwarf_Unsigned attribute::unsigned_constant () const {
        return debug_->attribute_form_unsigned_constant (att_);
    }
}

namespace dwarf {
    dwarf_ptr make_dwarf (Elf * const elf) {
        auto error = Dwarf_Error{nullptr};
        auto debug = Dwarf_Debug{nullptr};
        switch (::dwarf_elf_init (elf, DW_DLC_READ, nullptr, nullptr, &debug, &error)) {
        case DW_DLV_NO_ENTRY:
            assert (debug == nullptr);
            break;
        case DW_DLV_ERROR:
            throw dwarf_exception (debug, "dwarf_elf_init", error);
        case DW_DLV_OK:
            break;
        default:
            assert (0);
            break;
        }

        auto deleter = [](Dwarf_Debug d) {
            auto error = Dwarf_Error{nullptr};
            int res = ::dwarf_finish (d, &error);
            assert (res == DW_DLV_OK);
        };
        return {debug, deleter};
    }
}



namespace dwarf {
    bool is_type_reference_die (Dwarf_Half tag) {
        switch (tag) {
        case DW_TAG_pointer_type:
        case DW_TAG_reference_type:
        case DW_TAG_rvalue_reference_type:
        case DW_TAG_ptr_to_member_type:
        case DW_TAG_friend:
            return true;
        }
        return false;
    }

    bool debug::is_type_die (Dwarf_Half tag) {
        switch (tag) {
        case DW_TAG_structure_type:
        case DW_TAG_class_type:
        case DW_TAG_union_type:
        case DW_TAG_base_type:

        case DW_TAG_pointer_type:
        case DW_TAG_reference_type:
        case DW_TAG_rvalue_reference_type:
        case DW_TAG_ptr_to_member_type:
        case DW_TAG_friend:
            return true;
        }
        return false;
    }
}


namespace dwarf {
    // *******************
    // * owned_attribute *
    // *******************
    // (dtor)
    // ~~~~~~
    owned_attribute::~owned_attribute () {
        att_.get_debug ()->dealloc (att_.get_attribute (), DW_DLA_ATTR);
    }


    // ******************
    // * attribute_list *
    // ******************
    // (ctor)
    // ~~~~~~
    attribute_list::attribute_list (debug * debug, Dwarf_Die die) {
        Dwarf_Attribute * attributes{nullptr};
        Dwarf_Signed const size = debug->attrlist (die, &attributes);

        auto deleter = [debug](Dwarf_Attribute * p) { debug->dealloc (p, DW_DLA_LIST); };
        std::unique_ptr<Dwarf_Attribute, decltype (deleter)> at1 (attributes, deleter);

        array_.reserve (size);
        for (auto a = attributes, end = attributes + size; a < end; ++a) {
            array_.emplace_back (debug, *a);
        }
    }

    // (dtor)
    // ~~~~~~
    attribute_list::~attribute_list () {
        for (auto & a : array_) {
            a.get_debug ()->dealloc (a.get_attribute (), DW_DLA_ATTR);
        }
    }
}

// eof dwarf_helpers.cpp
