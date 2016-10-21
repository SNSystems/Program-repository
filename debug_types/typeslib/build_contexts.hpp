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

#ifndef BUILD_CONTEXTS_HPP
#define BUILD_CONTEXTS_HPP

#include <atomic>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lockfree/queue.hpp>

#include "dwarf_helpers.hpp"
#include <libdwarf.h>

using die_context_value = std::tuple<std::string, std::shared_ptr<std::string>>;
using die_context_map = std::unordered_map<Dwarf_Off, die_context_value>;
std::ostream & operator<< (std::ostream & os, die_context_map const & contexts);

namespace dwarf {
    class debug;
}

using context_queue = boost::lockfree::queue<Dwarf_Off>;

// **************************
// * context_queue_producer *
// **************************
unsigned context_queue_producer (boost::iostreams::mapped_file const & map_file,
                                 context_queue * queue /*out*/);
unsigned context_queue_producer (dwarf::debug * const debug, context_queue * queue /*out*/);
unsigned context_queue_producer (class cu_container & cus, context_queue * queue /*out*/);

template <typename Iterator>
unsigned context_queue_producer (dwarf::debug * const debug, Iterator first, Iterator last,
                                 context_queue * const queue /*out*/) {
    assert (queue != nullptr);
    auto num_cus = 0U;
    for (; first != last; ++first) {
        dwarf::die_ptr const & cu = *first;
        bool ok = queue->push (debug->die_to_offset (cu.get ()));
        assert (ok);

        ++num_cus;
    }
    return num_cus;
}


// ******************
// * build_contexts *
// ******************
class updater_intf;
class build_contexts {
public:
    build_contexts ();

    // No copying or assignment allowed.
    build_contexts (build_contexts const &) = delete;
    build_contexts & operator= (build_contexts const &) = delete;

    void consumer_thread (boost::iostreams::mapped_file const & map_file, context_queue & queue,
                          updater_intf & progress, std::atomic<bool> & error);

    void consumer (dwarf::debug * const debug, context_queue & queue, updater_intf & progress,
                   std::atomic<bool> & error);
    void consumer (boost::iostreams::mapped_file const & map_file, context_queue & queue,
                   updater_intf & progress, std::atomic<bool> & error);

    die_context_map && release_contexts () {
        return std::move (contexts_);
    }
    unsigned die_count () const {
        return die_count_;
    }

private:
    void record_die_context (dwarf::debug * const debug, Dwarf_Die die,
                             std::shared_ptr<std::string> const & producer, std::string ctx);
    std::shared_ptr<std::string> cu_at_producer (dwarf::debug * const debug,
                                                 dwarf::die_ptr const & cu_die);

    std::mutex contexts_mut_;
    die_context_map contexts_;

    std::atomic<unsigned> die_count_;
};
#endif // BUILD_CONTEXTS_HPP
// eof build_contexts.hpp
