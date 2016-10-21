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

#include "die_tree.hpp"
#include "progress.hpp"
#include "scan_type.hpp"
#include <array>
#include <gmock/gmock.h>
#include <tuple>
#include <utility>
#include <vector>

#include "as_hex.hpp"
#include <iostream>

namespace {
    class ScanType : public DieTree {
    protected:
        static void append (std::vector<std::uint8_t> & dest,
                            std::vector<std::uint8_t> const & src) {
            std::copy (std::begin (src), std::end (src), std::back_inserter (dest));
        }

        Dwarf_Die_s * add_die (Dwarf_Half tag, std::initializer_list<Dwarf_Attribute_s> && attrs) {
            dies_.emplace_back (tag, attrs);
            auto result = &dies_.back ();
            result->offset (offset_ += 10);
            return result;
        }
        Dwarf_Die_s * add_die (Dwarf_Half tag) {
            return add_die (tag, {});
        }

        void setup () {
            DieTree::setup (std::begin (dies_), std::end (dies_));
        }

        auto build_ExampleE_2_1_structC (Dwarf_Die_s * base_type_int = nullptr)
            -> std::tuple<Dwarf_Die, Dwarf_Die>;
        auto build_ExampleE_2_1_classA () -> std::tuple<Dwarf_Die, Dwarf_Die>;

        std::vector<Dwarf_Die_s> dies_;
        Dwarf_Off offset_ = 0;
        die_context_map contexts_;
    };


    // The base_type_int parameter to this test exists to allow the definition of 'int' to be shared
    // between the two compilation units that are built for 'struct C' and 'class A'. The exaple in
    // the
    // specification assumes that the producer knows that there's really only one definition of
    // 'int'
    // and that it can be referenced in the second type. Unfortunately, as a DWARF consumer, we
    // don't
    // know this and only have the DIE itself (which may of course be much more complex than 'int')
    // to go on.

    auto ScanType::build_ExampleE_2_1_structC (Dwarf_Die_s * base_type_int)
        -> std::tuple<Dwarf_Die, Dwarf_Die> {
        std::size_t const orig_capacity = dies_.capacity ();

        auto cu = add_die (DW_TAG_compile_unit, {{DW_AT_language, DW_LANG_C_plus_plus}});
        auto namespace_N = add_die (DW_TAG_namespace, {{DW_AT_name, "N"}});
        auto struct_C = add_die (
            DW_TAG_structure_type,
            {{DW_AT_name, "C"}, {DW_AT_byte_size, 8}, {DW_AT_decl_file, 1}, {DW_AT_decl_line, 5}});
        auto struct_C_member_x =
            add_die (DW_TAG_member, {{DW_AT_name, "x"},
                                     {DW_AT_decl_file, 1},
                                     {DW_AT_decl_line, 6},
                                     {DW_AT_data_member_location, Dwarf_Signed{0}}});
        auto struct_C_member_y = add_die (DW_TAG_member, {{DW_AT_name, "y"},
                                                          {DW_AT_decl_file, 1},
                                                          {DW_AT_decl_line, 7},
                                                          {DW_AT_data_member_location, 4}});

        // create an "int" DIE if there isn't one already.
        bool const int_in_this_cu = base_type_int == nullptr;
        if (int_in_this_cu) {
            base_type_int = add_die (
                DW_TAG_base_type,
                {{DW_AT_byte_size, 4}, {DW_AT_encoding, DW_ATE_signed}, {DW_AT_name, "int"}});
        }
        assert (dies_.size () <= orig_capacity);

        struct_C_member_x->add_attribute (DW_AT_type, base_type_int);
        struct_C_member_y->add_attribute (DW_AT_type, base_type_int);

        // clang-format off
        cu->child (namespace_N);
            namespace_N->sibling (int_in_this_cu ? base_type_int : nullptr).child (struct_C);
                struct_C->child (struct_C_member_x);
                    struct_C_member_x->sibling (struct_C_member_y);
                    // struct_C_member_y
            // base_type_int?
        // clang-format on

        auto const producer = std::make_shared<std::string> ("producer");
        contexts_[struct_C->offset ()] = std::make_tuple (std::string{"C9N"} + '\0', producer);
        contexts_[base_type_int->offset ()] = std::make_tuple (std::string{}, producer);
        return std::make_tuple (cu, struct_C);
    }

    auto ScanType::build_ExampleE_2_1_classA () -> std::tuple<Dwarf_Die, Dwarf_Die> {
        std::size_t const orig_capacity = dies_.capacity ();

        auto cu = add_die (DW_TAG_compile_unit, {{DW_AT_language, DW_LANG_C_plus_plus}});
        auto namespace_N = add_die (DW_TAG_namespace, {{DW_AT_name, "N"}});
        auto class_A = add_die (DW_TAG_class_type, {{DW_AT_name, "A"},
                                                    {DW_AT_byte_size, 20},
                                                    {DW_AT_decl_file, 1},
                                                    {DW_AT_decl_line, 10}});
        auto class_A_v_ = add_die (DW_TAG_member, {{DW_AT_name, "v_"},
                                                   {DW_AT_decl_file, 1},
                                                   {DW_AT_decl_line, 15},
                                                   {DW_AT_data_member_location, Dwarf_Signed{0}},
                                                   {DW_AT_accessibility, DW_ACCESS_private}});
        auto class_A_next = add_die (DW_TAG_member, {{DW_AT_name, "next"},
                                                     {DW_AT_decl_file, 1},
                                                     {DW_AT_decl_line, 16},
                                                     {DW_AT_data_member_location, 4},
                                                     {DW_AT_accessibility, DW_ACCESS_private}});
        auto class_A_bp = add_die (DW_TAG_member, {{DW_AT_name, "bp"},
                                                   {DW_AT_decl_file, 1},
                                                   {DW_AT_decl_line, 17},
                                                   {DW_AT_data_member_location, 8},
                                                   {DW_AT_accessibility, DW_ACCESS_private}});
        auto class_A_c = add_die (DW_TAG_member, {{DW_AT_name, "c"},
                                                  {DW_AT_decl_file, 1},
                                                  {DW_AT_decl_line, 18},
                                                  {DW_AT_data_member_location, 12},
                                                  {DW_AT_accessibility, DW_ACCESS_private}});
        auto class_A_A = add_die (DW_TAG_subprogram, {{DW_AT_external, 1},
                                                      {DW_AT_name, "A"},
                                                      {DW_AT_decl_file, 1},
                                                      {DW_AT_decl_line, 12},
                                                      {DW_AT_declaration, 1}});
        auto class_A_A_fp1 = add_die (DW_TAG_formal_parameter, {{DW_AT_artificial, 1}});
        auto class_A_A_fp2 = add_die (DW_TAG_formal_parameter);
        auto class_A_v = add_die (DW_TAG_subprogram, {{DW_AT_external, 1},
                                                      {DW_AT_name, "v"},
                                                      {DW_AT_decl_file, 1},
                                                      {DW_AT_decl_line, 13},
                                                      {DW_AT_declaration, 1}});
        auto class_A_v_fp1 = add_die (DW_TAG_formal_parameter, {{DW_AT_artificial, 1}});
        auto ptr_class_A = add_die (DW_TAG_pointer_type);
        auto ptr_struct_B = add_die (DW_TAG_pointer_type);
        auto namespace_N2 = add_die (DW_TAG_namespace, {{DW_AT_name, "N"}});
        auto struct_B =
            add_die (DW_TAG_structure_type, {{DW_AT_name, "B"}, {DW_AT_declaration, 1}});
        auto base_type_int =
            add_die (DW_TAG_base_type,
                     {{DW_AT_byte_size, 4}, {DW_AT_encoding, DW_ATE_signed}, {DW_AT_name, "int"}});

        Dwarf_Die_s * cu2;
        Dwarf_Die_s * struct_C;
        std::tie (cu2, struct_C) = build_ExampleE_2_1_structC (base_type_int);

        // Check that adding the DIEs didn't invalidate the pointers.
        assert (dies_.size () <= orig_capacity);

        auto L1 = class_A; // Some DIE labels to match those used in the DWARF spec.
        auto L2 = base_type_int;
        auto L3 = ptr_class_A;
        auto L4 = ptr_struct_B;
        auto L5 = struct_B;

        // Connect up the type references.
        class_A_v_->add_attribute (DW_AT_type, L2);
        class_A_next->add_attribute (DW_AT_type, L3);
        class_A_bp->add_attribute (DW_AT_type, L4);
        class_A_c->add_attribute (DW_AT_type, struct_C);
        class_A_A_fp1->add_attribute (DW_AT_type, L3);
        class_A_A_fp2->add_attribute (DW_AT_type, L2);
        class_A_v_fp1->add_attribute (DW_AT_type, L3);
        ptr_class_A->add_attribute (DW_AT_type, L1);
        ptr_struct_B->add_attribute (DW_AT_type, L5);

        // clang-format off
        // Establish the DIE tree.
        cu->sibling (cu2).child (namespace_N);
            namespace_N->sibling (ptr_class_A).child (class_A);
                class_A->child (class_A_v_);
                    class_A_v_->sibling (class_A_next);
                    class_A_next->sibling (class_A_bp);
                    class_A_bp->sibling (class_A_c);
                    class_A_c->sibling (class_A_A);
                    class_A_A->sibling (class_A_v).child (class_A_A_fp1);
                        class_A_A_fp1->sibling (class_A_A_fp2);
                        //class_A_A_fp2
                    class_A_v->child (class_A_v_fp1);
                        //class_A_v_fp1
            ptr_class_A->sibling (ptr_struct_B);
            ptr_struct_B->sibling (namespace_N2);
            namespace_N2->sibling (base_type_int).child(struct_B);
                //struct B
            //base_type_int
        // clang-format on

        auto producer = std::make_shared<std::string> ("producer");
        contexts_[class_A->offset ()] = std::make_tuple (std::string{"C9N"} + '\0', producer);
        contexts_[base_type_int->offset ()] = std::make_tuple (std::string{}, producer);
        contexts_[ptr_class_A->offset ()] = std::make_tuple (std::string{}, producer);
        contexts_[ptr_struct_B->offset ()] = std::make_tuple (std::string{}, producer);
        contexts_[struct_B->offset ()] = std::make_tuple (std::string{"C9N"} + '\0', producer);
        return std::make_tuple (cu, class_A);
    }
}


TEST_F (ScanType, ExampleE_2_1_structC) {
    dies_.reserve (6);

    auto cu_struct_C = this->build_ExampleE_2_1_structC ();
    this->setup ();

    constexpr std::size_t reserve = 63;
    std::vector<std::uint8_t> expected;
    expected.reserve (reserve);

    // clang-format off
    // Step 2: 'C' DW_TAG_namespace "N"
    append (expected, {0x43, 0x39, 0x4e, 0x00});
    // Step 3: 'D' DW_TAG_structure_type
    append (expected, {0x44, 0x13});
    // Step 4: 'A' DW_AT_name DW_FORM_string "C"
    append (expected, {0x41, 0x03, 0x08, 0x43, 0x00});
    // Step 4: 'A' DW_AT_byte_size DW_FORM_sdata 8
    append (expected, {0x41, 0x0b, 0x0d, 0x08});
    // Step 7: First child ("x")
        // Step 3: 'D' DW_TAG_member
        append (expected, {0x44, 0x0d});
        //Step 4: 'A' DW_AT_name DW_FORM_string "x"
        append (expected, {0x41, 0x03, 0x08, 0x78, 0x00});
        // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 0
        append (expected, {0x41, 0x38, 0x0d, 0x00});
        // Step 6: 'T' DW_AT_type (type #2)
        append (expected, {0x54, 0x49});
            // Step 3: 'D' DW_TAG_base_type
            append (expected, {0x44, 0x24});
            // Step 4: 'A' DW_AT_name DW_FORM_string "int"
            append (expected, {0x41, 0x03, 0x08, 0x69, 0x6e, 0x74, 0x00});
            // Step 4: 'A' DW_AT_byte_size DW_FORM_sdata 4
            append (expected, {0x41, 0x0b, 0x0d, 0x04});
            // Step 4: 'A' DW_A_encoding DW_FORM_sdata DW_ATE_signed
            append (expected, {0x41, 0x3e, 0x0d, 0x05});
            // Step 7: End of DW_TAG_base_type "int"
            append (expected, {0x00});
        // Step 7: End of DW_TAG_member "x"
        append (expected, {0x00});
    // Step 7: Second child ("y")
        // Step 3: 'D' DW_TAG_member
        append (expected, {0x44, 0x0d});
        // Step 4: 'A' DW_AT_name DW_FORM_string "y"
        append (expected, {0x41, 0x03, 0x08, 0x79, 0x00});
        // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 4
        append (expected, {0x41, 0x38, 0x0d, 0x04});
        // Step 6: 'R' DW_AT_type (type #2)
        append (expected, {0x52, 0x49, 0x02});
        // Step 7: End of DW_TAG_member "y"
        append (expected, {0x00});
    // Step 7: End of DW_TAG_structure_type "C"
    append (expected, {0x00});
    // clang-format on

    std::vector<std::uint8_t> S;
    S.reserve (reserve);
    Dwarf_Die struct_C = std::get<1> (cu_struct_C);
    scan_type (&debug, struct_C, contexts_, std::back_inserter (S));
    EXPECT_THAT (S, ::testing::ContainerEq (expected));
    EXPECT_EQ (reserve, S.size ());
}


TEST_F (ScanType, ExampleE_2_1_classA) {
    dies_.reserve (23);

    auto cu_class_A = this->build_ExampleE_2_1_classA ();
    this->setup ();

    constexpr std::size_t reserve = 189;
    std::vector<std::uint8_t> expected;
    expected.reserve (reserve);

    // clang-format off
    // Step 2: 'C' DW_TAG_namespace "N"
    append (expected, {0x43, 0x39, 0x4e, 0x00});
    // Step 3: 'D' DW_TAG_class_type
    append (expected, {0x44, 0x02});
    // Step 4: 'A' DW_AT_name DW_FORM_string "A"
    append (expected, {0x41, 0x03, 0x08, 0x41, 0x00});
    // Step 4: 'A' DW_AT_byte_size DW_FORM_sdata 20
    append (expected, {0x41, 0x0b, 0x0d, 0x14});
    // Step 7: First child ("v_")
        // Step 3: 'D' DW_TAG_member
        append (expected, {0x44, 0x0d});
        // Step 4: 'A' DW_AT_name DW_FORM_string "v_"
        append (expected, {0x41, 0x03, 0x08, 0x76, 0x5f, 0x00});
        // Step 4: 'A' DW_AT_accessibility DW_FORM_sdata DW_ACCESS_private
        append (expected, {0x41, 0x32, 0x0d, 0x03});
        // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 0
        append (expected, {0x41, 0x38, 0x0d, 0x00});
        // Step 6: 'T' DW_AT_type (type #2)
        append (expected, {0x54, 0x49});
            // Step 3: 'D' DW_TAG_base_type
            append (expected, {0x44, 0x24});
            // Step 4: 'A' DW_AT_name DW_FORM_string "int"
            append (expected, {0x41, 0x03, 0x08, 0x69, 0x6e, 0x74, 0x00});
            // Step 4: 'A' DW_AT_byte_size DW_FORM_sdata DW_ATE_signed
            append (expected, {0x41, 0x0b, 0x0d, 0x04});
            // Step 4: 'A' DW_AT_encoding DW_FORM_sdata DW_ATE_signed
            append (expected, {0x41, 0x3e, 0x0d, 0x05});
            // Step 7: End of DW_TAG_base_type "int"
            append (expected, {0x00});
        // Step 7: End of DW_TAG_member "v_"
        append (expected, {0x00});
    // Step 7: Second child ("next")
        // Step 3: 'D' DW_TAG_member
        append (expected, {0x44, 0x0d});
        // Step 4: 'A' DW_AT_name DW_FOM_string "next"
        append (expected, {0x41, 0x03, 0x08, 0x6e, 0x65, 0x78, 0x74, 0x00});
        // Step 4: 'A' DW_AT_accessibility DW_FORM_sdata DW_ACCESS_private
        append (expected, {0x41, 0x32, 0x0d, 0x03});
        // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 4
        append (expected, {0x41, 0x38, 0x0d, 0x04});
        // Step 6: 'T' DW_AT_type (type #3)
        append (expected, {0x54, 0x49});
            // Step 3: 'D' DW_TAG_pointer_type
            append (expected, {0x44, 0x0f});
            // Step 5: 'N' DW_AT_type
            append (expected, {0x4e, 0x49});
            // Step 5: 'C' DW_AT_namespace "N" 'E'
            append (expected, {0x43, 0x39, 0x4e, 0x00, 0x45});
            // Step 5: "A"
            append (expected, {0x41, 0x00});
            // Step 7: End of DW_TAG_pointer_type
            append (expected, {0x00});
        // Step 7: End of DW_TAG_member "next"
        append (expected, {0x00});
    // Step 7: Third child ("bp")
        // Step 3: 'D' DW_TAG_member
        append (expected, {0x44, 0x0d});
        // Step 4: 'A' DW_AT_name DW_FORM_string "bp"
        append (expected, {0x41, 0x03, 0x08, 0x62, 0x70, 0x00});
        // Step 4: 'A' DW_AT_accessibility DW_FORM_sdata DW_ACCESS_rivate
        append (expected, {0x41, 0x32, 0x0d, 0x03});
        // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 8
        append (expected, {0x41, 0x38, 0x0d, 0x08});
        // Step 6: 'T' DW_AT_type (type #4)
        append (expected, {0x54, 0x49});
            // Step 3: 'D' DW_TAG_pointer_type
            append (expected, {0x44, 0x0f});
            // Step 5: 'N' DW_AT_type
            append (expected, {0x4e, 0x49});
            // Step 5: 'C' DW_AT_namespace "N" 'E'
            append (expected, {0x43, 0x39, 0x4e, 0x00, 0x45});
            // Step 5: "B"
            append (expected, {0x42, 0x00});
            // Step 7: End of DW_TAG_pointer_type
            append (expected, {0x00});
        // Step 7: End of DW_TAG_member "bp"
        append (expected, {0x00});
    // Step 7: Fourth child ("c")
        // Step 3: 'D' DW_TAG_member
        append (expected, {0x44, 0x0d});
        // Step 4: 'A' DW_AT_name DW_FORM_string "c"
        append (expected, {0x41, 0x03, 0x08, 0x63, 0x00});
        // Step 4: 'A' DW_AT_accessibility DW_FORM_sdata DW_ACCESS_private
        append (expected, {0x41, 0x32, 0x0d, 0x03});
        // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 12
        append (expected, {0x41, 0x38, 0x0d, 0x0c});
        // Step 6: 'T' DW_AT_type (type #5)
        append (expected, {0x54, 0x49});
            // Step 2: 'C' DW_TAG_namespace "N"
            append (expected, {0x43, 0x39, 0x4e, 0x00});
            // Step 3: 'D' DW_TAG_structure_type
            append (expected, {0x44, 0x13});
            // Step 4: 'A' DW_AT_name DW_form_string "C"
            append (expected, {0x41, 0x03, 0x08, 0x43, 0x00});
            // Step 4: 'A' DW_AT_byte-size DW_FORM_sdata 8
            append (expected, {0x41, 0x0b, 0x0d, 0x08});
            // Step 7: First child ("x")
                // Step 3: 'D' DW_TAG_member
                append (expected, {0x44, 0x0d});
                // Step 4: 'A' DW_AT_name DW_FORM_string "x"
                append (expected, {0x41, 0x03, 0x08, 0x78, 0x00});
                // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 0
                append (expected, {0x41, 0x38, 0x0d, 0x00});
                // Step 6: 'R' DW_AT_type (type #2)
                append (expected, {0x52, 0x49, 0x02});
                // Step 7: End of DW_TAG_member "x"
                append (expected, {0x00});
            // Step 7: Second child ("y")
                // Step 3: 'D' DW_TAG_member
                append (expected, {0x44, 0x0d});
                // Step 4: 'A' DW_AT_name DW_FORM_string "y"
                append (expected, {0x41, 0x03, 0x08, 0x79, 0x00});
                // Step 4: 'A' DW_AT_data_member_location DW_FORM_sdata 4
                append (expected, {0x41, 0x38, 0x0d, 0x04});
                // Step 6: 'R' DW_AT_type (type #2)
                append (expected, {0x52, 0x49, 0x02});
                // Step 7: End of DW_TAG_member "y"
                append (expected, {0x00});
            // Step 7: End of DW_TAG_structure_type "C"
            append (expected, {0x00});
        // Step 7: End of DW_TAG_member "c"
        append (expected, {0x00});
    // Step 7: Fifth child ("A")
        // Step 3: 'S' DW_TAG_subprogram "A"
        append (expected, {0x53, 0x2e, 0x41, 0x00});
    // Step 7: Sixth child ("v")
        // Step 3: 'S' DW_TAG_subprogram "v"
        append (expected, {0x53, 0x2e, 0x76, 0x00});
    // Step 7: End of DW_TAG_structure_type "A"
    append (expected, {0x00});
    // clang-format on

    std::vector<std::uint8_t> S;
    Dwarf_Die class_A = std::get<1> (cu_class_A);
    scan_type (&debug, class_A, contexts_, std::back_inserter (S));
    EXPECT_EQ (expected.size (), S.size ());
#if 0
    // This is handy for finding the first difference if the test should ever fail!
    {
        bool different = false;
        for (std::size_t ctr = 0, last = std::min (expected.size (), S.size ()); ctr < last; ++ctr) {
            auto const x = expected [ctr];
            auto const y = S [ctr];
            if (x != y) {
                different = true;
            }
            if (different) {
                std::cout << ctr << ": " << as_hex (x) << ' ' << as_hex (y);
                if (x != y) {
                    std::cout << " *";
                }
                std::cout << '\n';
            }
        }
    }
#endif
    EXPECT_THAT (S, ::testing::ContainerEq (expected));
    EXPECT_EQ (reserve, S.size ());
}
// eof test_scan_type.cpp
