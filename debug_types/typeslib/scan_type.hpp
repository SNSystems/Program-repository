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

#ifndef SCAN_TYPE_HPP
#define SCAN_TYPE_HPP

#include <dwarf.h>
#include <libdwarf.h>

#include "append.hpp"
#include "build_contexts.hpp"
#include "dwarf_helpers.hpp"
#include "iter_pair.hpp"
#include "leb128.hpp"
#include "sort_attributes.hpp"
#include "visited_types.hpp"

dwarf::die_ptr die_referenced_by_attribute (dwarf::attribute const & attribute);
dwarf::die_ptr type_die_referenced_by_attribute (dwarf::attribute const & attribute);



namespace details {
    template <typename SequenceIterator>
    SequenceIterator scan_type_body (dwarf::debug * const debug, Dwarf_Die die, SequenceIterator S,
                                     visited_types & V, die_context_map const & contexts);

    // Implements the part of step 4 for an attribute that refers to another type entry. That is...
    //
    // An attribute that refers to another type entry T is processed as
    // follows:
    //
    // (a) If T is in the list V at some V[x], use the letter 'R' as the
    //     marker and use the unsigned LEB128 encoding of x as the attribute
    //     value; otherwise,
    // (b) use the letter 'T' as the marker, process the type T recursively
    //     by performing Steps 2 through 7, and use the result as the
    //     attribute value.

    template <typename SequenceIterator>
    SequenceIterator
    process_attribute_reference_to_type (dwarf::debug * const debug, Dwarf_Die ref_die,
                                         dwarf::attribute const & attribute, SequenceIterator S,
                                         visited_types & V, die_context_map const & contexts) {
        auto const tag = attribute.what_attr ();

        visited_types::const_iterator ref_die_it = V.find (ref_die);
        if (ref_die_it != V.end ()) {
            // "If T is in the list V at some V[x], use the letter 'R' as
            // the marker and use the unsigned LEB128 encoding of x as the
            // attribute value.

            *(S++) = 'R';
            S = encode_uleb128 (tag, S);
            S = encode_uleb128 (ref_die_it->second, S);
        } else {
            *(S++) = 'T';
            S = encode_uleb128 (tag, S);

            // ref_die is a reference to a separate type, so it (or rather
            // its offset) gets recorded in the set of visited types.
            V.add (ref_die);

            S = scan_type_body (debug, ref_die, S, V, contexts);
        }
        return S;
    }


    template <typename SequenceIterator, typename AttributeList = dwarf::attribute_list>
    SequenceIterator scan_type_body2 (dwarf::debug * const debug, Dwarf_Die die, SequenceIterator S,
                                      visited_types & V, die_context_map const & contexts) {
        // print_cout ("scan_type_body2 @", debug->die_to_offset (die), " name:", debug->name
        // (die));

        // 3. Append to S the letter 'D', followed by the DWARF tag of the
        //     debugging information entry...

        Dwarf_Half const die_tag = debug->tag (die);
        {
            *(S++) = 'D';
            S = encode_uleb128 (die_tag, S);
        }

        // 4. For each of the attributes that are present in the DIE, in the
        // order given by ordered_attributes[], append to S a marker letter
        // (see below), the DWARF attribute code, and the attribute value.
        //
        // An attribute that refers to another type entry T is processed as
        // follows:
        //
        // (a) If T is in the list V at some V[x], use the letter 'R' as the
        //     marker and use the unsigned LEB128 encoding of x as the attribute
        //     value; otherwise,
        // (b) use the letter 'T' as the marker, process the type T recursively
        //     by performing Steps 2 through 7, and use the result as the
        //     attribute value.
        //
        // Other attribute values use the letter 'A' as the marker, and the
        // value consists of the form code (encoded as an unsigned LEB128 value)
        // followed by the encoding of the value according to the form code. To
        // ensure reproducibility of the signature, the set of forms used in the
        // signature computation is limited to the following: DW_FORM_sdata,
        // DW_FORM_flag, DW_FORM_string, and DW_FORM_block.

        {
            AttributeList attributes (debug, die);
            auto first = std::begin (attributes);
            auto last = sort_attributes (first, std::end (attributes));
            for (auto const & attribute : make_range (first, last)) {
                if (dwarf::die_ptr ref_die = type_die_referenced_by_attribute (attribute)) {
                    S = process_attribute_reference_to_type (debug, ref_die.get (), attribute, S, V,
                                                             contexts);
                } else {
                    S = append_attribute (attribute, S);
                }
            }
        }


        // 5. If the tag in Step 3 is one of DW_TAG_pointer_type,
        //    DW_TAG_reference_type, DW_TAG_rvalue_reference_type,
        //    DW_TAG_ptr_to_member_type, or DW_TAG_friend, and the referenced
        //    type (via the DW_AT_type or DW_AT_friend attribute) has a
        //    DW_AT_name attribute, append to S the letter 'N', the DWARF
        //    attribute code (DW_AT_type or DW_AT_friend), the context of the
        //    type (according to the method in Step 2), the letter 'E', and the
        //    name of the type. For DW_TAG_friend, if the referenced entry is a
        //    DW_TAG_subprogram, the context is omitted and the name to be used
        //    is the ABI-specific name of the subprogram (e.g., the mangled
        //    linker name).
        //
        // 6. If the tag in Step 3 is not one of DW_TAG_pointer_type,
        //    DW_TAG_reference_type, DW_TAG_rvalue_reference_type,
        //    DW_TAG_ptr_to_member_type, or DW_TAG_friend, but has a DW_AT_type
        //    attribute, or if the referenced type (via the DW_AT_type or
        //    DW_AT_friend attribute) does not have a DW_AT_name attribute,
        //    the attribute is processed according to the method in Step 4 for
        //    an attribute that refers to another type entry.

        if (dwarf::is_type_reference_die (die_tag)) {
            Dwarf_Half const type_reference_tag =
                (die_tag == DW_TAG_friend) ? DW_AT_friend : DW_AT_type;
            // FIXME: friend handling is not properly implemented.
            if (dwarf::owned_attribute reference_attribute =
                    debug->attribute_from_tag (die, type_reference_tag)) {
                if (dwarf::die_ptr ref_die =
                        die_referenced_by_attribute (reference_attribute.get ())) {
                    auto const name = debug->name (ref_die.get ());
                    if (name.length () > 0) {
                        *(S++) = 'N';
                        S = encode_uleb128 (type_reference_tag, S);
                        S = append_type_die_context (debug, ref_die.get (), S, contexts);
                        *(S++) = 'E';
                        S = append_string (name, S);
                    } else {
                        // step 4.
                        S = process_attribute_reference_to_type (
                            debug, ref_die.get (), reference_attribute.get (), S, V, contexts);
                    }
                } else {
                    // GCC sometimes elides DW_AT_TYPE field from a DW_TAG_pointer DIE to represent
                    // void*.
                    // FIXME: Is doing nothing enough or do I need to simulate the presence of a
                    // DW_TAG_unspecified_type DIE? That would make pointer to missing type match
                    // pointer to void.
                }
            }
        } else {
            if (dwarf::owned_attribute at_type = debug->attribute_from_tag (die, DW_AT_type)) {
                dwarf::die_ptr ref_die = die_referenced_by_attribute (at_type.get ());
                assert (ref_die);
                // step 4
                S = process_attribute_reference_to_type (debug, ref_die.get (), at_type.get (), S,
                                                         V, contexts);
            }
        }


        // 7. Visit each child C of the debugging information entry as follows:
        //    If C is a nested type entry or a member function entry, and has a
        //    DW_AT_name attribute, append to S the letter'S", the tag of C,
        //    and its name; otherwise, process C recursively by performing Steps
        //    3 through 7, appending the result to S. Following the last child
        //    (or if there are no children), append a zero byte.

        for (dwarf::die_ptr & C : dwarf::die_children (debug, die)) {
            auto const child_tag = debug->tag (C.get ());
            bool handled = false;
            bool const is_type = debug->is_type_die (child_tag);
            if (is_type || child_tag == DW_TAG_subprogram) {
                auto const name = debug->name (C.get ());
                if (name.length () > 0) {
                    *(S++) = 'S';
                    S = encode_uleb128 (child_tag, S);
                    S = append_string (name, S);
                    handled = true;
                }
            }
            if (!handled) {
                // Skip nested types. They'll be picked up by the outer loop.
                if (!is_type) {
                    S = scan_type_body2 (debug, C.get (), S, V, contexts);
                }
            }
        }

        *(S++) = '\0';
        return S;
    }

    template <typename SequenceIterator>
    SequenceIterator scan_type_body (dwarf::debug * const debug, Dwarf_Die die, SequenceIterator S,
                                     visited_types & V, die_context_map const & contexts) {
        // 2. If the debugging information entry represents a type that is
        //    nested inside another type or a namespace, append to S the type's
        //    context...

        S = append_type_die_context (debug, die, S, contexts);
        return scan_type_body2 (debug, die, S, V, contexts);
    }
}

template <typename OutputIterator>
OutputIterator scan_type (dwarf::debug * const debug, Dwarf_Die die,
                          die_context_map const & contexts, OutputIterator S) {
    assert (debug->is_type_die (die));

    // 1. Start with an empty sequence S [that's in the hands of the caller] and a list V of visted
    // types,
    //    where V is initialized to a list containing the type T0 as its single element. Elements in
    //    V are
    //    indexed from 1, so that V[1] is T0.
    visited_types V (debug);
    V.add (die);
    return details::scan_type_body (debug, die, S, V, contexts);
}

#endif // SCAN_TYPE_HPP
// eof scan_type.hpp
