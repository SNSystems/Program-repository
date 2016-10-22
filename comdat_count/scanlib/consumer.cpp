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

#include "consumer.hpp"

// Standard library includes
#include <iostream>
#include <mutex>
#include <sstream>

// 3rd party includes
#include <boost/iostreams/device/mapped_file.hpp>

// Local includes
#include "comdat_scanner.hpp"
#include "elf_enumerator.hpp"
#include "elf_helpers.hpp"
#include "flags.hpp"
#include "print.hpp"
#include "progress.hpp"
#include "temp_files.hpp"
#include "zipper.hpp"


// consumer
// ~~~~~~~~
// Thread entry-point.
void consumer (queue_type & queue, comdat_scanner * const scanner, output_flags const & ofl,
               state_flags * const state, updater & progress) {

    assert (scanner != nullptr);
    assert (state != nullptr);

    queue_member * qmem = nullptr;
    while (queue.pop (qmem)) {
        // If an error has been raised, then we need to end this thread.
        if (state->error) {
            break;
        }

        try {
            if (qmem == nullptr) {
                throw std::runtime_error ("Cannot process an empty path.");
            }

            auto const & file_path = qmem->real_path;
            auto const & zip_member_name = qmem->member_name;
            auto const & user_file_path = qmem->user_path;

            path_cleanup pathc (file_path, false);

            if (zip_member_name.length () > 0) {
                boost::filesystem::path const & zip_path = pathc.path ();
                zipper::zip_ptr uf (::unzOpen64 (zip_path.string ().c_str ()), &::unzClose);
                if (uf.get () == nullptr) {
                    std::ostringstream str;
                    str << "Unable to open zip file " << pathc.path ();
                    throw std::runtime_error (str.str ());
                }
                // copy the zip member into a temporary file and set path to its locations
                pathc = path_cleanup (temporary_file_path (), true);
                zipper::extract (uf.get (), zip_member_name.c_str (), pathc.path (),
                                 user_file_path);
            }


            if (ofl.verbose) {
                print_cout ("Processing: ", user_file_path);
            }

            if (file_size (pathc.path ()) == 0) {
                // Skip zero size files.
                if (!ofl.quiet) {
                    print_cout ("Skipping: ", user_file_path);
                }
                continue;
            }

            enumerate (pathc.path (), user_file_path, scanner, &progress);
        } catch (std::exception const & ex) {
            // Tell the other threads that we've encountered an error and bail.
            state->error = true;
            print_cerr ("An error occurred: ", ex.what ());
            break;
        } catch (...) {
            // Tell the other threads that we've encountered an error and bail.
            state->error = true;
            print_cerr ("Oh dear. An unknown exception occurred.");
            break;
        }
    }
}
// eof scanlib/consumer.cpp
