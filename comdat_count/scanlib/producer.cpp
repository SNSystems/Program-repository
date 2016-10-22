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

#include "producer.hpp"

#include <iostream>
#include <list>

#include "consumer.hpp"
#include "flags.hpp"
#include "print.hpp"
#include "zipper.hpp"

namespace {

    std::mutex zip_members_lock;
    std::list<queue_member> member_storage;
}

std::size_t push_zip_contents (unzFile uf, boost::filesystem::path const & zip_path,
                               queue_type & queue) {
    std::size_t num_queued = 0;
    std::lock_guard<std::mutex> guard (zip_members_lock);
    int err = UNZ_OK;
    for (err = unzGoToFirstFile (uf); err == UNZ_OK; err = unzGoToNextFile (uf)) {

        static constexpr std::size_t const buffer_elements = 256;
        char filename_inzip[buffer_elements];

        unz_file_info64 file_info;
        err = unzGetCurrentFileInfo64 (uf, &file_info, filename_inzip, buffer_elements, nullptr,
                                       0,        // extra field/extra field buffer size
                                       NULL, 0); // comment/comment buffer size
        if (err != UNZ_OK) {
            zipper::throw_unzip_error (err, zip_path);
        }

        filename_inzip[buffer_elements - 1] = '\0';
        member_storage.push_back (
            {zip_path, std::string{filename_inzip}, zip_path / filename_inzip});
        queue.push (&member_storage.back ());
        ++num_queued;
    }
    if (err != UNZ_END_OF_LIST_OF_FILE) {
        zipper::throw_unzip_error (err, zip_path);
    }
    return num_queued;
}


namespace {
    std::size_t path_processor (queue_type & queue, boost::filesystem::path const & p) {
        std::size_t num_queued = 0;
        zipper::zip_ptr uf = zipper::open (p, std::nothrow);
        if (uf) {
            num_queued += push_zip_contents (uf.get (), p, queue);
        } else {
            std::lock_guard<std::mutex> guard (zip_members_lock);
            member_storage.push_back ({p, std::string (), p});
            queue.push (&member_storage.back ());
            ++num_queued;
        }
        return num_queued;
    }

    // A file name is hidden if it's the current or parent directory (. and ..)
    // or begins with a '.'
    bool file_is_hidden (boost::filesystem::path const & p) {
        auto const & name = p.string ();
        return name != ".." && name != "." && name[0] == '.';
    }
}


std::size_t queue_input_files (queue_type & queue, std::vector<std::string> const & file_paths,
                               output_flags const & ofl) {

    std::size_t num_queued = 0;

    // Push the input files into the queue.
    for (boost::filesystem::path const & path : file_paths) {
        if (!boost::filesystem::is_directory (path)) {
            num_queued += path_processor (queue, path);
        } else {
            if (ofl.verbose) {
                print_cout ("Scanning: ", path);
            }

            for (auto it = boost::filesystem::recursive_directory_iterator (path),
                      end = boost::filesystem::recursive_directory_iterator{};
                 it != end; ++it) {

                boost::filesystem::path const p = *it;
                bool const is_hidden = file_is_hidden (p);
                if (boost::filesystem::is_directory (p)) {
                    if (boost::filesystem::is_symlink (p) || is_hidden) {
                        it.no_push (); // don't recurse into this path.
                    } else {
                        if (ofl.verbose) {
                            print_cout ("Scanning: ", p);
                        }
                    }
                } else {
                    if (!is_hidden) {
                        num_queued += path_processor (queue, p);
                    }
                }
            }
        }
    }
    return num_queued;
}
// eof scanlib/producer.cpp
