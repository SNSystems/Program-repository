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

#include <cassert>

#include <dwarf.h>

#include "append.hpp"
#include "cu_iterator.hpp"
#include "dwarf_exception.hpp"
#include "dwarf_helpers.hpp"
#include "elf_helpers.hpp"
#include "leb128.hpp"
#include "print.hpp"
#include "progress.hpp"
#include "worker_error.hpp"


// ***********************************************************
// * operator<< (std::ostream & os, die_context_map const &) *
// ***********************************************************
std::ostream & operator<< (std::ostream & os, die_context_map const & contexts) {
    char const * indent = "    ";
    char const * sep = "[\n";
    for (auto c : contexts) {
        auto const & offset = c.first;
        auto const & context = std::get<0> (c.second);
        auto const & producer = std::get<1> (c.second);
        os << sep << indent << "{ \"offset\": " << offset << ", \"context\": \"" << context
           << ", \"producer\": \"" << *producer << "\" }";
        sep = ",\n";
    }
    return os << "\n]\n";
}


// **************************
// * context_queue_producer *
// **************************
unsigned context_queue_producer (boost::iostreams::mapped_file const & map_file,
                                 context_queue * const queue /*out*/) {
    auto elf = elf::make_elf (map_file.const_data (), map_file.size ());
    auto debug_ptr = dwarf::make_dwarf (elf);
    dwarf::debug debug (debug_ptr);
    return context_queue_producer (&debug, queue);
}

unsigned context_queue_producer (dwarf::debug * const debug, context_queue * const queue /*out*/) {
    cu_container cus (debug);
    return context_queue_producer (cus, queue);
}

unsigned context_queue_producer (cu_container & cus, context_queue * const queue /*out*/) {
    return context_queue_producer (cus.debug (), std::begin (cus), std::end (cus), queue);
}


// ******************
// * build_contexts *
// ******************
// (ctor)
// ~~~~~~
build_contexts::build_contexts ()
        : die_count_ (0) {}

// cu_at_producer
// ~~~~~~~~~~~~~~
// Returns the producer name from a CU DIE.
std::shared_ptr<std::string> build_contexts::cu_at_producer (dwarf::debug * const debug,
                                                             dwarf::die_ptr const & cu_die) {
    if (dwarf::owned_attribute att = debug->attribute_from_tag (cu_die.get (), DW_AT_producer)) {
        return std::make_shared<std::string> (att.get ().string ());
    }
    return std::make_shared<std::string> ("unknown");
}

// consumer
// ~~~~~~~~
void build_contexts::consumer (dwarf::debug * const debug, context_queue & queue,
                               updater_intf & progress, std::atomic<bool> & error) {
    auto cu = Dwarf_Off{0};
    while (queue.pop (cu)) {
        if (error) {
            // Stop if another thread threw an error.
            break;
        }
        // print_cout ("Building contexts for CU @", cu);
        auto cu_die = debug->offset_to_die (cu);
        record_die_context (debug, cu_die.get (), cu_at_producer (debug, cu_die), std::string{});

        progress.completed_incr ();
    }
}

void build_contexts::consumer (boost::iostreams::mapped_file const & map_file,
                               context_queue & queue, updater_intf & progress,
                               std::atomic<bool> & error) {
    auto elf = elf::make_elf (map_file.const_data (), map_file.size ());
    auto debug_ptr = dwarf::make_dwarf (elf);
    dwarf::debug debug (debug_ptr);
    this->consumer (&debug, queue, progress, error);
}


// consumer_thread
// ~~~~~~~~~~~~~~~
void build_contexts::consumer_thread (boost::iostreams::mapped_file const & map_file,
                                      context_queue & queue, updater_intf & progress,
                                      std::atomic<bool> & error) {
    try {
        this->consumer (map_file, queue, progress, error);
    } catch (dwarf_exception const & dex) {
        print_cerr ("DWARF exception: ", dex);
        error = true;
    } catch (std::exception const & ex) {
        print_cerr ("Exception: ", ex.what ());
        error = true;
    } catch (...) {
        print_cerr ("Unknown exception");
        error = true;
    }
}

// record_die_context
// ~~~~~~~~~~~~~~~~~~
void build_contexts::record_die_context (dwarf::debug * const debug, Dwarf_Die die,
                                         std::shared_ptr<std::string> const & producer,
                                         std::string ctx) {

    // 2. If the debugging information entry represents a type that is
    //    nested inside another type or a namespace, append to S the type's
    //    context as follows: For each surrounding type or namespace,
    //    beginning with the outermost such construct, append the letter
    //    'C', the DWARF tag of the construct, and the name (taken from the
    //    DW_AT_name attribute) of the type or namespace (including its
    //    trailing null byte).

    auto const tag = debug->tag (die);
    bool const is_type = debug->is_type_die (tag);
    ++die_count_;

    if (is_type) {
        auto const offset = debug->die_to_offset (die);
        std::lock_guard<std::mutex> guard (contexts_mut_);
        assert (contexts_.find (offset) == contexts_.end ());
        contexts_.emplace (offset, std::make_tuple (ctx, producer));
    }

    if (tag == DW_TAG_namespace || is_type) {
        auto inserter = std::back_inserter (ctx);
        *(inserter++) = 'C';
        inserter = encode_uleb128 (debug->tag (die), inserter);
        inserter = append_string (debug->name (die), inserter);
    }

    for (auto & child_die : dwarf::die_children (debug, die)) {
        record_die_context (debug, child_die.get (), producer, ctx);
    }
}
// eof build_contexts.cpp
