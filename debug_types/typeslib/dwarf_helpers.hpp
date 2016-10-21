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

#ifndef DWARF_HELPERS_H
#define DWARF_HELPERS_H

#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>
#include <libdwarf.h>

#include "elf_helpers.hpp"

namespace dwarf {
    using dwarf_ptr =
        std::unique_ptr<std::remove_pointer<Dwarf_Debug>::type, std::function<void(Dwarf_Debug)>>;

    dwarf_ptr make_dwarf (Elf * const elf);
    inline dwarf_ptr make_dwarf (elf::elf_ptr const & elf) {
        return make_dwarf (elf.get ());
    }


    using die_ptr =
        std::unique_ptr<std::remove_pointer<Dwarf_Die>::type, std::function<void(Dwarf_Die)>>;

    bool is_type_reference_die (Dwarf_Half tag);
}


namespace dwarf {
    class debug;
    // *************
    // * attribute *
    // *************
    class attribute {
    public:
        attribute (debug * const debug, Dwarf_Attribute att)
                : debug_ (debug)
                , att_ (att) { // No that att may be nullptr.
        }
        attribute (attribute const &) = default;
        attribute & operator= (attribute const &) = default;

        operator bool () const {
            return att_ != nullptr;
        }

        debug * get_debug () const {
            return debug_;
        }
        Dwarf_Attribute get_attribute () const {
            return att_;
        }

        Dwarf_Half what_attr () const;
        Dwarf_Half what_form () const;

        // Attribute forms
        bool flag () const;
        Dwarf_Off ref () const;
        std::string string () const;
        using block_ptr = std::unique_ptr<Dwarf_Block, std::function<void(Dwarf_Block *)>>;
        block_ptr block () const;
        Dwarf_Signed signed_constant () const;
        Dwarf_Unsigned unsigned_constant () const;

    private:
        debug * debug_;
        Dwarf_Attribute att_;
    };


    // *******************
    // * owned_attribute *
    // *******************
    class owned_attribute {
    public:
        owned_attribute (debug * const debug, Dwarf_Attribute att)
                : att_ (debug, att) {}
        ~owned_attribute ();

        attribute & get () {
            return att_;
        }
        operator bool () const {
            return att_;
        }

    private:
        attribute att_;
    };


    // *********
    // * debug *
    // *********
    class debug {
    public:
        explicit debug (Dwarf_Debug debug)
                : debug_ (debug) {}
        explicit debug (dwarf_ptr const & debug)
                : debug_ (debug.get ()) {}
        virtual ~debug () {}

        Dwarf_Debug get () {
            return debug_;
        }

        virtual bool next_cu_header (Dwarf_Unsigned * cu_header_length,       // out
                                     Dwarf_Half * version_stamp,              // out
                                     Dwarf_Off * abbrev_offset,               // out
                                     Dwarf_Half * address_size,               // out
                                     Dwarf_Unsigned * next_cu_header_offset); // out

        virtual Dwarf_Off die_to_offset (Dwarf_Die die);
        virtual die_ptr offset_to_die (Dwarf_Off offset);
        virtual void dealloc (Dwarf_Ptr space, Dwarf_Unsigned alloc_type) {
            ::dwarf_dealloc (debug_, space, alloc_type);
        }
        virtual die_ptr as_die_ptr (Dwarf_Die die) {
            return {die, [this](Dwarf_Die d) { this->dealloc (d, DW_DLA_DIE); }};
        }

        // Related DIEs
        virtual die_ptr siblingof (Dwarf_Die die);
        virtual die_ptr child (Dwarf_Die die);

        // DIE properties.
        virtual Dwarf_Half tag (Dwarf_Die die);
        static bool is_type_die (Dwarf_Half tag);
        bool is_type_die (Dwarf_Die die) {
            return this->is_type_die (this->tag (die));
        }
        bool is_type_die (dwarf::die_ptr const & die) {
            return this->is_type_die (die.get ());
        }
        virtual std::string name (Dwarf_Die die);
        virtual Dwarf_Signed attrlist (Dwarf_Die die, Dwarf_Attribute ** array);

        // Atrributes
        virtual dwarf::owned_attribute attribute_from_tag (Dwarf_Die die, Dwarf_Half tag);

        virtual Dwarf_Half attribute_what_attr (Dwarf_Attribute a) const;
        virtual Dwarf_Half attribute_what_form (Dwarf_Attribute a) const;

        // Attribute forms
        virtual std::string attribute_form_string (Dwarf_Attribute a) const;
        virtual bool
        attribute_form_flag (Dwarf_Attribute a) const; // FIXME: they shouldn't be const.
        virtual Dwarf_Signed attribute_form_signed_constant (Dwarf_Attribute a) const;
        virtual Dwarf_Unsigned attribute_form_unsigned_constant (Dwarf_Attribute a) const;
        virtual attribute::block_ptr attribute_form_block (Dwarf_Attribute a);
        virtual Dwarf_Off attribute_form_ref (Dwarf_Attribute a);

    private:
        Dwarf_Debug debug_;
    };


    // ****************
    // * die_children *
    // ****************
    class die_children {
    public:
        class iterator : public boost::iterator_facade<iterator, dwarf::die_ptr,
                                                       boost::forward_traversal_tag> {
        public:
            explicit iterator (debug * const debug)
                    : debug_ (debug)
                    , die_ (debug->as_die_ptr (nullptr)) {}
            iterator (debug * const debug, dwarf::die_ptr && die)
                    : debug_ (debug)
                    , die_ (std::move (die)) {}

        private:
            friend class boost::iterator_core_access;

            /// Navigates to the next DIE.
            void increment () {
                die_ = debug_->siblingof (die_.get ());
            }

            /// Compares two iterators.
            bool equal (iterator const & other) const {
                return debug_ == other.debug_ && die_ == other.die_;
            }

            /// Access the value referred to by the iterator.
            dwarf::die_ptr & dereference () const {
                return die_;
            }

            debug * debug_;
            mutable dwarf::die_ptr die_;
        };

        die_children (debug * const debug, Dwarf_Die parent)
                : debug_ (debug)
                , parent_ (parent) {}

        // No copying or assignment allowed.
        die_children (die_children const &) = delete;
        die_children & operator= (die_children const &) = delete;

        iterator begin () {
            return iterator (debug_, debug_->child (parent_));
        }
        iterator end () {
            return iterator (debug_);
        }

    private:
        debug * debug_;
        Dwarf_Die parent_;
    };


    // ******************
    // * attribute_list *
    // ******************
    class attribute_list {
    public:
        using container_type = std::vector<attribute>;
        using value_type = container_type::value_type;
        using iterator = container_type::iterator;

        attribute_list (debug * debug, Dwarf_Die die);
        explicit attribute_list (container_type && c)
                : array_ (std::move (c)) {}
        ~attribute_list ();

        // No copying or assignment allowed.
        attribute_list (attribute_list const &) = delete;
        attribute_list & operator= (attribute_list const &) = delete;

        iterator begin () {
            return std::begin (array_);
        }
        iterator end () {
            return std::end (array_);
        }
        std::size_t size () const {
            return array_.size ();
        }

    private:
        container_type array_;
    };
}

#endif // DWARF_HELPERS_H

// eof dwarf_helpers.h
