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

#include <boost/thread.hpp>

#include <dwarf.h>
#include <libdwarf.h>

#include "build_contexts.hpp"
#include "counts.hpp"
#include "dwarf_exception.hpp"
#include "dwarf_helpers.hpp"
#include "function_output_iterator.hpp"
#include "md5.h"
#include "options.hpp"
#include "print.hpp"
#include "progress.hpp"
#include "scan_type.hpp"
#include "worker_error.hpp"

namespace {
    template <typename Container>
    std::ostream & dump_container (std::ostream & os, Container const & S) {
#if 0
        static_assert (std::is_same <typename Container::value_type, std::uint8_t>::value, "S must be a container of unsigned bytes");
        os << "sequence:\n";
        char const * sep = "";
        for (std::uint8_t c: S) {
            os << sep << "0x" << as_hex (c);
            sep = " ";
        }
        return os << "\n";
#else
        return os;
#endif
    }


    std::uint64_t get_sequence_digest (md5::hasher & md5) {
        auto digest = md5.finish ();
        return (static_cast<std::uint64_t> (digest[15]) << 0) |
               (static_cast<std::uint64_t> (digest[14]) << 8) |
               (static_cast<std::uint64_t> (digest[13]) << 16) |
               (static_cast<std::uint64_t> (digest[12]) << 24) |
               (static_cast<std::uint64_t> (digest[11]) << 32) |
               (static_cast<std::uint64_t> (digest[10]) << 40) |
               (static_cast<std::uint64_t> (digest[9]) << 48) |
               (static_cast<std::uint64_t> (digest[8]) << 56);
    }

    template <typename InputIterator>
    std::uint64_t get_sequence_digest (InputIterator first, InputIterator last) {
        md5::hasher md5;
        md5.append (first, last);
        return get_sequence_digest (md5);
    }



    std::uint64_t scan_type (dwarf::debug * const debug, Dwarf_Die die,
                             die_context_map const & contexts) {
#if CAPTURE_SEQUENCE
        // 1. Start with an empty sequence S and a list V of visited types, where V is initialized
        // to a list
        // containing the type T0 as its single element. Elements in V are indexed from 1, so that
        // V[1] is T0.
        std::vector<std::uint8_t> S;
        auto sequence = std::back_inserter (S);
#else
        md5::hasher S;
        std::function<void(std::uint8_t)> hash_appender = [&S](std::uint8_t x) { S.append (x); };
        auto sequence = make_function_output_iterator (hash_appender);
#endif

        scan_type (debug, die, contexts, sequence);

#if CAPTURE_SEQUENCE
        dump_container (std::cout, S);
        return get_sequence_digest (std::begin (S), std::end (S));
#else
        return get_sequence_digest (S);
#endif
    }
}


namespace {

    using context_map_range_queue =
        boost::lockfree::queue<iter_pair<die_context_map::const_iterator> *,
                               boost::lockfree::fixed_sized<true>>;

    // ********************
    // * scan_dies_thread *
    // ********************
    void scan_dies_thread (boost::iostreams::mapped_file const & map_file,
                           context_map_range_queue & work_queue, die_context_map const & contexts,
                           counts & c, updater_intf & progress, std::atomic<bool> & error) {
        try {
            ::elf_errno (); // reset the ELF error code for this thread

            auto elf = elf::make_elf (map_file.const_data (), map_file.size ());
            assert (elf::is_elf (elf) && elf::kind (elf) != ELF_K_AR);
            auto debug_ptr = dwarf::make_dwarf (elf);
            dwarf::debug debug (debug_ptr.get ());

            auto range = context_map_range_queue::value_type{nullptr};
            while (work_queue.pop (range)) {
                if (error) {
                    // Stop if another thread threw an error.
                    break;
                }

                for (auto const & offset_info : *range) {
                    Dwarf_Off const offset = offset_info.first;
                    die_context_value const & value = offset_info.second;

                    auto die = debug.offset_to_die (offset);
                    assert (debug.is_type_die (die));

                    auto const signature = scan_type (&debug, die.get (), contexts);
                    c.add_type (signature, *std::get<1> (value));

                    progress.completed_incr ();
                }
            }
        } catch (dwarf_exception const & dex) {
            print_cerr ("DWARF error: ", dex);
            error = true;
        } catch (std::exception const & ex) {
            print_cerr ("Error: ", ex.what ());
            error = true;
        } catch (...) {
            print_cerr ("Unknown exception");
            error = true;
        }
    }


    // *****************
    // * create_thread *
    // *****************
    template <typename Function>
    void create_thread (boost::thread_group & threads, Function function) {
        boost::thread_attributes attrs;
        attrs.set_stack_size (std::size_t{8} * 1024 * 1024);

        auto t = std::unique_ptr<boost::thread> (new boost::thread (attrs, function));
        threads.add_thread (t.get ());
        t.release ();
    }


    // *******************
    // * updater_factory *
    // *******************
    class updater_factory {
    public:
        explicit updater_factory (bool enabled)
                : enabled_ (enabled) {}

        std::unique_ptr<updater_intf> create (char const * message) const {
            return std::unique_ptr<updater_intf> (
                enabled_ ? static_cast<updater_intf *> (new updater (message))
                         : static_cast<updater_intf *> (new silent_updater (message)));
        }

    private:
        bool enabled_;
    };


    // **********************
    // * build_die_contexts *
    // **********************
    auto build_die_contexts (boost::iostreams::mapped_file const & map_file,
                             unsigned const num_threads, updater_factory const & uf)
        -> std::pair<die_context_map, unsigned> {

        context_queue queue (1000); // a lock-free queue primed for 1000 CUs.
        auto const num_cus = context_queue_producer (map_file, &queue);

        build_contexts builder;
        {
            auto progress = uf.create ("Phase 1/2: Discovering type DIEs");
            progress->total (num_cus);
            progress->run ();

            std::atomic<bool> error{false};
            boost::thread_group threads;
            for (auto thread_count = 0U; thread_count < num_threads; ++thread_count) {
                auto entry_point =
                    std::bind (&build_contexts::consumer_thread, &builder, std::cref (map_file),
                               std::ref (queue), std::ref (*progress), std::ref (error));
                create_thread (threads, entry_point);
            }
            threads.join_all ();
            if (error) {
                throw worker_error ();
            }
        }
        return std::make_pair (builder.release_contexts (), builder.die_count ());
    }


    // ******************
    // * scan_type_dies *
    // ******************
    counts scan_type_dies (boost::iostreams::mapped_file const & map_file,
                           die_context_map const & contexts, unsigned total_dies,
                           unsigned num_threads, updater_factory const & uf) {
        auto progress = uf.create ("Phase 2/2: Scanning type DIEs");
        progress->total (contexts.size ());
        progress->run ();

        constexpr auto types_per_work_packet = std::size_t{100};
        std::size_t const queue_members = contexts.size () / types_per_work_packet + 1U;
        context_map_range_queue queue (queue_members);
        std::vector<std::remove_pointer<context_map_range_queue::value_type>::type> queue_storage;
        queue_storage.reserve (queue_members);
        {
            auto first = std::begin (contexts);
            auto last = first;
            auto elements = contexts.size ();
            while (elements > 0) {
                auto slice = std::min (elements, types_per_work_packet);
                std::advance (last, slice);
                queue_storage.emplace_back (first, last);
                bool ok = queue.push (&queue_storage.back ());
                assert (ok);
                first = last;

                elements -= slice;
            }
            assert (first == std::end (contexts));
        }

        counts result (total_dies);
        {
            std::atomic<bool> error{false};
            boost::thread_group threads;
            for (auto thread_count = 0U; thread_count < num_threads; ++thread_count) {
                auto entry_point =
                    std::bind (scan_dies_thread, map_file, std::ref (queue), std::cref (contexts),
                               std::ref (result), std::ref (*progress), std::ref (error));
                create_thread (threads, entry_point);
            }
            // Wait for the worker threads to finish.
            threads.join_all ();
            if (error) {
                throw worker_error ();
            }
        }
        return result;
    }
}



// ****************
// * process_file *
// ****************
void process_file (options const & options) {
    boost::iostreams::mapped_file map_file (options.input_path, std::ios::in);
    updater_factory updater (options.progress_enabled);

    die_context_map contexts;
    unsigned total_dies;
    std::tie (contexts, total_dies) = build_die_contexts (map_file, options.threads, updater);

    if (options::output_file_opener contexts_file = options.contexts_file ()) {
        *contexts_file << contexts;
    }

    counts const c = scan_type_dies (map_file, contexts, total_dies, options.threads, updater);
    if (options::output_file_opener ofo = options.count_output_file ()) {
        *ofo << c;
    }
}
// eof process_file.cpp
