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

#ifndef SCANLIB_CONSUMER_HPP
#define SCANLIB_CONSUMER_HPP

// Standard library includes
#include <string>

// 3rd party includes
#include <boost/filesystem.hpp>
#include <boost/lockfree/queue.hpp>


// The job queue
struct queue_member {
    boost::filesystem::path real_path;
    std::string member_name;
    boost::filesystem::path user_path;
};
using queue_type = boost::lockfree::queue<queue_member const *>;


class comdat_scanner;
struct output_flags;
struct state_flags;
class updater;
void consumer (queue_type & queue, comdat_scanner * const scanner, output_flags const & ofl,
               state_flags * const state, updater & progress);

#endif // SCANLIB_CONSUMER_HPP
// eof scanlib/consumer.hpp
