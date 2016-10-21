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

#include "process_file.hpp"

#include <array>
#include <string>

#include "append.hpp"
#include "build_contexts.hpp"
#include "mock_debug.hpp"
#include <gmock/gmock.h>

namespace {
    class AppendContext : public ::testing::Test {
    public:
        void SetUp () {
            using ::testing::Return;
            ON_CALL (debug, die_to_offset (die.get ())).WillByDefault (Return (offset));
        }

    protected:
        ::testing::NiceMock<mock_debug> debug;
        dwarf::die_ptr die = debug.new_die ();
        Dwarf_Off const offset = 100;
        std::string const context = "C9ns";
    };
}

TEST_F (AppendContext, HasContext) {
    std::string S;
    die_context_map contexts{
        {offset, std::make_tuple (context, std::make_shared<std::string> ("producer"))}};
    append_type_die_context (&debug, die.get (), std::back_inserter (S), contexts);
    EXPECT_EQ (context, S);
}

TEST_F (AppendContext, NoContext) {
    std::string S;

    die_context_map contexts;
    append_type_die_context (&debug, die.get (), std::back_inserter (S), contexts);
    EXPECT_EQ ("", S);
}


namespace {
    class AppendAttribute : public ::testing::Test {
    public:
        AppendAttribute ()
                : raw (1, 1)
                , attribute (&debug, &raw) {}

    protected:
        ::testing::NiceMock<mock_debug> debug;
        Dwarf_Attribute_s raw;
        ::dwarf::attribute attribute;

        void append_compare (std::vector<std::uint8_t> const & container) {
            std::vector<std::uint8_t> S;
            append_attribute (attribute, std::back_inserter (S));
            EXPECT_THAT (S, ::testing::ContainerEq (container));
        }
    };
}

TEST_F (AppendAttribute, String) {
    using ::testing::Return;
    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_name));
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_string));
    ON_CALL (debug, attribute_form_string (&raw)).WillByDefault (Return ("hello"));
    this->append_compare ({'A', 0x03, 0x08, 'h', 'e', 'l', 'l', 'o', 0});
}

TEST_F (AppendAttribute, FlagTrue) {
    using ::testing::Return;
    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_elemental));
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_flag));
    ON_CALL (debug, attribute_form_flag (&raw)).WillByDefault (Return (true));
    this->append_compare ({'A', 0x66, 0x0c, 1});
}

TEST_F (AppendAttribute, FlagFalse) {
    using ::testing::Return;
    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_use_UTF8));
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_flag));
    ON_CALL (debug, attribute_form_flag (&raw)).WillByDefault (Return (false));
    this->append_compare ({'A', 0x53, 0x0c, 0});
}

TEST_F (AppendAttribute, FlagPresent) {
    using ::testing::Return;
    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_declaration));
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_flag_present));
    EXPECT_CALL (debug, attribute_form_flag (&raw)).Times (0);
    this->append_compare ({'A', 0x3c, 0x0c, 1});
}

TEST_F (AppendAttribute, Unsigned) {
    using ::testing::Return;

    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_decl_file));
    ON_CALL (debug, attribute_form_unsigned_constant (&raw)).WillByDefault (Return (42U));

    std::vector<std::uint8_t> const expected{'A', 0x3a, 0x0d, 42};
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_udata));
    this->append_compare (expected);

    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_data1));
    this->append_compare (expected);

    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_data2));
    this->append_compare (expected);

    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_data4));
    this->append_compare (expected);

    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_data8));
    this->append_compare (expected);
}

TEST_F (AppendAttribute, Signed) {
    using ::testing::Return;
    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_decl_file));
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_sdata));
    ON_CALL (debug, attribute_form_signed_constant (&raw)).WillByDefault (Return (-1));

    std::vector<std::uint8_t> const expected{'A', 0x3a, 0x0d, 0x7f};
    this->append_compare (expected);
}


TEST_F (AppendAttribute, Block) {
    using ::testing::Invoke;
    using ::testing::Return;

    std::array<std::uint8_t, 2> data{{42, 43}};

    ON_CALL (debug, attribute_what_attr (&raw)).WillByDefault (Return (DW_AT_decl_file));
    ON_CALL (debug, attribute_what_form (&raw)).WillByDefault (Return (DW_FORM_block));
    ON_CALL (debug, get_attribute_form_block (&raw))
        .WillByDefault (Invoke ([&data](Dwarf_Attribute) -> Dwarf_Block * {
            std::unique_ptr<Dwarf_Block> result (new Dwarf_Block);
            result->bl_len = sizeof (data);
            result->bl_data = data.data ();
            result->bl_from_loclist = 0;
            result->bl_section_offset = 0;
            return result.release ();
        }));
    std::vector<std::uint8_t> expected{'A', 0x3a /*DW_AT_decl_file*/, 0x09 /*DW_FORM_block*/};
    std::copy (std::begin (data), std::end (data), std::back_inserter (expected));
    this->append_compare (expected);
}
// eof test_process_file.cpp
