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

#ifndef DIE_TREE_HPP
#define DIE_TREE_HPP

#include "leb128.hpp"
#include "mock_debug.hpp"
#include <gmock/gmock.h>
#include <map>

class DieTree : public ::testing::Test {
protected:
    template <typename DieIterator>
    void setup (DieIterator first, DieIterator last) {
        using ::testing::Invoke;
        using ::testing::_;

        off_map_.clear ();
        std::for_each (first, last, [this](Dwarf_Die_s & d) {
            auto const offset = d.offset ();
            assert (off_map_.find (offset) == off_map_.end ()); // offsets must be unique
            off_map_.emplace (offset, &d);
        });

        // Methods which establish offset <-> die relationship.
        ON_CALL (debug, die_to_offset (_))
            .WillByDefault (Invoke ([](Dwarf_Die d) { return d->offset (); }));
        ON_CALL (debug, get_offset_to_die (_))
            .WillByDefault (Invoke ([this](Dwarf_Off offset) {
                auto it = off_map_.find (offset);
                assert (it != std::end (off_map_));
                return new Dwarf_Die_s (*(it->second));
            }));

        // Methods for nativating the child/sibling tree.
        auto get_relative = [](Dwarf_Die_s const * d) {
            return d == nullptr ? nullptr : new Dwarf_Die_s (*d);
        };
        ON_CALL (debug, get_child (_))
            .WillByDefault (
                Invoke ([&get_relative](Dwarf_Die d) { return get_relative (d->child ()); }));
        ON_CALL (debug, get_siblingof (_))
            .WillByDefault (
                Invoke ([&get_relative](Dwarf_Die d) { return get_relative (d->sibling ()); }));

        // Methods for accessing DIE properties.
        ON_CALL (debug, tag (_)).WillByDefault (Invoke ([](Dwarf_Die d) { return d->tag (); }));
        ON_CALL (debug, name (_))
            .WillByDefault (Invoke ([](Dwarf_Die d) {
                auto it = std::find_if (
                    std::begin (*d), std::end (*d),
                    [](Dwarf_Attribute_s const & att) { return att.tag == DW_AT_name; });
                return it == std::end (*d) ? std::string{} : it->str;
            }));

        // Methods for accessing the DIE's attributes collection.
        ON_CALL (debug, attrlist (_, _))
            .WillByDefault (Invoke ([](Dwarf_Die d, Dwarf_Attribute ** array) {
                *array = d->empty () ? nullptr : d->attrlist ();
                return static_cast<Dwarf_Signed> (d->size ());
            }));
        ON_CALL (debug, attribute_from_tag (_, _))
            .WillByDefault (Invoke (
                [this](Dwarf_Die d, Dwarf_Half tag) { return this->attribute_from_tag (d, tag); }));

        // Attribute properties.
        ON_CALL (debug, attribute_what_attr (_))
            .WillByDefault (Invoke ([](Dwarf_Attribute a) { return a->tag; }));
        ON_CALL (debug, attribute_what_form (_))
            .WillByDefault (Invoke ([](Dwarf_Attribute a) { return a->form; }));

        // Attribute forms
        ON_CALL (debug, attribute_form_ref (_))
            .WillByDefault (Invoke ([](Dwarf_Attribute a) {
                EXPECT_EQ (DW_FORM_ref_udata, a->form);
                return a->ref->offset ();
            }));
        ON_CALL (debug, attribute_form_string (_))
            .WillByDefault (Invoke ([](Dwarf_Attribute a) {
                EXPECT_EQ (DW_FORM_string, a->form);
                return a->str;
            }));
        ON_CALL (debug, attribute_form_signed_constant (_))
            .WillByDefault (Invoke ([](Dwarf_Attribute a) {
                EXPECT_EQ (DW_FORM_sdata, a->form);
                return a->s;
            }));
        ON_CALL (debug, attribute_form_unsigned_constant (_))
            .WillByDefault (Invoke ([](Dwarf_Attribute a) {
                EXPECT_EQ (DW_FORM_udata, a->form);
                return static_cast<Dwarf_Unsigned> (a->s);
            }));
    }

    static std::string uleb128 (std::uint64_t v) {
        std::string result;
        encode_uleb128 (v, std::back_inserter (result));
        return result;
    }

    ::testing::NiceMock<mock_debug> debug;

private:
    std::map<Dwarf_Off, Dwarf_Die_s const *> off_map_;

    dwarf::owned_attribute attribute_from_tag (Dwarf_Die d, Dwarf_Half tag) {
        auto predicate = [tag](Dwarf_Attribute_s const & att) { return att.tag == tag; };
        auto it = std::find_if (std::begin (*d), std::end (*d), predicate);
        return dwarf::owned_attribute (&debug,
                                       it == std::end (*d) ? nullptr : new Dwarf_Attribute_s (*it));
    }
};

#endif // DIE_TREE_HPP
// eof die_tree.hpp
