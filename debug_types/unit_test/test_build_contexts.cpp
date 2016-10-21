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

#include "build_contexts.hpp"

#include <array>
#include <vector>

#include "die_tree.hpp"
#include "leb128.hpp"
#include "progress.hpp"
#include <dwarf.h>
#include <gmock/gmock.h>

namespace {
    template <typename ValueType, typename OutputIterator>
    OutputIterator copy_queue (boost::lockfree::queue<ValueType> & queue, OutputIterator out) {
        ValueType v;
        while (queue.pop (v)) {
            *(out++) = v;
        }
        return out;
    }
}

TEST (ContextQueueProducer, EmptyCUContainer) {
    context_queue queue (1);

    mock_debug debug;
    std::vector<dwarf::die_ptr> cu_dies;
    unsigned const cus =
        context_queue_producer (&debug, std::begin (cu_dies), std::end (cu_dies), &queue);
    EXPECT_EQ (0U, cus);

    std::vector<Dwarf_Off> result;
    copy_queue (queue, std::back_inserter (result));
    EXPECT_EQ (0U, result.size ());
}

TEST (ContextQueueProducer, TwoCUs) {
    context_queue queue (1);

    using ::testing::Return;
    using ::testing::ElementsAre;

    mock_debug debug;

    // Simulate the CU iterator yielding a range which hold two CU DIEs at offsets
    // cu0off and cu1off respectively.
    std::array<dwarf::die_ptr, 2> cu_dies{{debug.new_die (), debug.new_die ()}};

    static constexpr auto cu0off =
        Dwarf_Off{101}; // (There is no particular significance to these numbers.)
    static constexpr auto cu1off = Dwarf_Off{211};
    EXPECT_CALL (debug, die_to_offset (cu_dies[0].get ())).WillRepeatedly (Return (cu0off));
    EXPECT_CALL (debug, die_to_offset (cu_dies[1].get ())).WillRepeatedly (Return (cu1off));

    // Turn the CU range into a queue of CU DIE offsets to be recursively scaned.
    unsigned const cus =
        context_queue_producer (&debug, std::begin (cu_dies), std::end (cu_dies), &queue);
    EXPECT_EQ (2U, cus);

    std::vector<Dwarf_Off> result;
    copy_queue (queue, std::back_inserter (result));
    EXPECT_THAT (result, ElementsAre (cu0off, cu1off));
}

namespace {
    class ContextQueueConsumer : public DieTree {};
}

TEST_F (ContextQueueConsumer, NoTypes) {
    // +10 DW_TAG_compile_unit
    //   +20 DW_TAG_lo_user
    // +30 DW_TAG_compile_unit
    std::array<Dwarf_Die_s, 3> dies;
    dies[0] = {Dwarf_Off{10}, DW_TAG_compile_unit, &dies[2] /*sibling*/, &dies[1] /*child*/};
    dies[1] = {Dwarf_Off{20}, DW_TAG_lo_user};
    dies[2] = {Dwarf_Off{30}, DW_TAG_compile_unit};
    this->setup (std::begin (dies), std::end (dies));

    context_queue queue (2);
    queue.push (dies[0].offset ());
    queue.push (dies[2].offset ());

    build_contexts builder;
    silent_updater progress;
    std::atomic<bool> error{false};
    builder.consumer (&debug, queue, progress, error);

    die_context_map const contexts = builder.release_contexts ();
    EXPECT_EQ (0U, contexts.size ());
    EXPECT_EQ (dies.size (), builder.die_count ());
    EXPECT_EQ (std::atomic<bool>{false}, error);
}


MATCHER_P (ContextMatches, n, "") {
    *result_listener << "where the context was \"" << std::get<0> (arg) << "\" and producer was \""
                     << *std::get<1> (arg) << "\"";
    return std::get<0> (arg) == std::get<0> (n) && *std::get<1> (arg) == std::get<1> (n);
}


TEST_F (ContextQueueConsumer, BaseType) {
    // +10 DW_TAG_compile_unit
    //   +20 DW_TAG_base_type "int"
    // +30 DW_TAG_compile_unit
    using ::testing::UnorderedElementsAre;
    using ::testing::Pair;

    std::array<Dwarf_Die_s, 3> dies;
    dies[0] = {Dwarf_Off{10},
               DW_TAG_compile_unit,
               &dies[2] /*sibling*/,
               &dies[1] /*child*/,
               {{DW_AT_producer, "producer"}}};
    dies[1] = {Dwarf_Off{20}, DW_TAG_base_type, {{DW_AT_name, "int"}}};
    dies[2] = {Dwarf_Off{30}, DW_TAG_compile_unit};
    this->setup (std::begin (dies), std::end (dies));

    context_queue queue (2);
    queue.push (dies[0].offset ());
    queue.push (dies[2].offset ());

    build_contexts builder;
    silent_updater progress;
    std::atomic<bool> error{false};
    builder.consumer (&debug, queue, progress, error);

    die_context_map const contexts = builder.release_contexts ();

    EXPECT_THAT (contexts, UnorderedElementsAre (
                               Pair (20, ContextMatches (std::make_tuple ("", "producer")))));
    EXPECT_EQ (dies.size (), builder.die_count ());
    EXPECT_EQ (std::atomic<bool>{false}, error);
}

TEST_F (ContextQueueConsumer, NestedStructsInNamespace) {
    // +10 DW_TAG_compile_unit
    //   +20 DW_TAG_namespace "ns"
    //     +30 DW_TAG_structure_type "foo"
    //       +40 DW_TAG_structure_type "bar"
    using ::testing::UnorderedElementsAre;
    using ::testing::Pair;

    std::array<Dwarf_Die_s, 4> dies;
    dies.at (0) = {Dwarf_Off{10}, DW_TAG_compile_unit, nullptr /*sibling*/, &dies.at (1) /*child*/};
    dies.at (1) = {Dwarf_Off{20},
                   DW_TAG_namespace,
                   nullptr /*sibling*/,
                   &dies.at (2) /*child*/,
                   {{DW_AT_name, "ns"}}};
    dies.at (2) = {Dwarf_Off{30},
                   DW_TAG_structure_type,
                   nullptr /*sibling*/,
                   &dies.at (3) /*child*/,
                   {{DW_AT_name, "foo"}}};
    dies.at (3) = {Dwarf_Off{40},
                   DW_TAG_structure_type,
                   nullptr /*sibling*/,
                   nullptr /*child*/,
                   {{DW_AT_name, "bar"}}};
    this->setup (std::begin (dies), std::end (dies));

    context_queue queue (1);
    queue.push (dies[0].offset ());

    silent_updater updater;
    build_contexts builder;
    std::atomic<bool> error{false};
    builder.consumer (&debug, queue, updater, error);

    die_context_map const contexts = builder.release_contexts ();

    auto const ns_context = std::string{"C9ns"} + '\0';
    auto const foo_context =
        ns_context + 'C' + uleb128 (DW_TAG_structure_type) + std::string{"foo"} + '\0';
    EXPECT_THAT (
        contexts,
        UnorderedElementsAre (
            Pair (Dwarf_Off{30}, ContextMatches (std::make_tuple (ns_context, "unknown"))),
            Pair (Dwarf_Off{40}, ContextMatches (std::make_tuple (foo_context, "unknown")))));
    EXPECT_EQ (dies.size (), builder.die_count ());
    EXPECT_EQ (std::atomic<bool>{false}, error);
}

TEST_F (ContextQueueConsumer, OneStructInEachOfTwoNamespaces) {
    // +10 DW_TAG_compile_unit
    //   +20 DW_TAG_namespace "ns1"
    //     +30 DW_TAG_structure_type "foo"
    //   +40 DW_TAG_namespace "ns2"
    //     +50 DW_TAG_structure_type "bar"

    using ::testing::UnorderedElementsAre;
    using ::testing::Pair;

    std::array<Dwarf_Die_s, 5> dies;
    dies.at (0) = {Dwarf_Off{10},
                   DW_TAG_compile_unit,
                   nullptr /*sibling*/,
                   &dies.at (1) /*child*/,
                   {{DW_AT_producer, "producer"}}};
    dies.at (1) = {Dwarf_Off{20},
                   DW_TAG_namespace,
                   &dies.at (3) /*sibling*/,
                   &dies.at (2) /*child*/,
                   {{DW_AT_name, "ns1"}}};
    dies.at (2) = {Dwarf_Off{30},
                   DW_TAG_structure_type,
                   nullptr /*sibling*/,
                   nullptr /*child*/,
                   {{DW_AT_name, "foo"}}};
    dies.at (3) = {Dwarf_Off{40},
                   DW_TAG_namespace,
                   nullptr /*sibling*/,
                   &dies.at (4) /*child*/,
                   {{DW_AT_name, "ns2"}}};
    dies.at (4) = {Dwarf_Off{50},
                   DW_TAG_structure_type,
                   nullptr /*sibling*/,
                   nullptr /*child*/,
                   {{DW_AT_name, "bar"}}};
    this->setup (std::begin (dies), std::end (dies));

    context_queue queue (1);
    queue.push (dies[0].offset ());

    silent_updater updater;
    build_contexts builder;
    std::atomic<bool> error{false};
    builder.consumer (&debug, queue, updater, error);

    die_context_map const contexts = builder.release_contexts ();

    auto const ns1_context = std::string{"C9ns1"} + '\0';
    auto const ns2_context = std::string{"C9ns2"} + '\0';

    EXPECT_THAT (
        contexts,
        UnorderedElementsAre (
            Pair (Dwarf_Off{30}, ContextMatches (std::make_tuple (ns1_context, "producer"))),
            Pair (Dwarf_Off{50}, ContextMatches (std::make_tuple (ns2_context, "producer")))));
    EXPECT_EQ (dies.size (), builder.die_count ());
    EXPECT_EQ (std::atomic<bool>{false}, error);
}


TEST_F (ContextQueueConsumer, BaseTypeErrorIndicated) {
    // +10 DW_TAG_compile_unit
    //   +20 DW_TAG_base_type "int"
    // +30 DW_TAG_compile_unit
    constexpr std::size_t reserve = 2;
    std::vector<Dwarf_Die_s> dies;
    dies.reserve (reserve);
    dies.emplace_back (Dwarf_Off{10}, DW_TAG_compile_unit);
    auto cu = &dies.back ();
    dies.emplace_back (Dwarf_Off{20}, DW_TAG_base_type, nullptr, nullptr,
                       std::initializer_list<Dwarf_Attribute_s>{{DW_AT_name, "int"}});
    auto base_type = &dies.back ();
    assert (dies.size () <= reserve);

    cu->child (base_type);

    this->setup (std::begin (dies), std::end (dies));

    context_queue queue (1);
    queue.push (dies.at (0).offset ());

    build_contexts builder;
    silent_updater progress;
    std::atomic<bool> error{true};
    builder.consumer (&debug, queue, progress, error);

    die_context_map const contexts = builder.release_contexts ();
    EXPECT_EQ (0U, contexts.size ());
    EXPECT_EQ (0U, builder.die_count ());
    EXPECT_EQ (std::atomic<bool>{true}, error);
}

// eof test_build_contexts.cpp
