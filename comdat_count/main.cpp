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

// Standard library includes
#include <cassert>
#include <exception>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

// 3rd party includes
#include <boost/filesystem.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/thread/thread.hpp>

// scanlib includes
#include "comdat_scanner.hpp"
#include "consumer.hpp"
#include "elf_helpers.hpp"
#include "options.hpp"
#include "producer.hpp"
#include "progress.hpp"
#include "temp_files.hpp"
#include "zipper.hpp"




namespace boost {
    void tss_cleanup_implemented () {
    }
}

namespace {
    std::unique_ptr <std::ofstream> output_file (std::string const & output) {
        std::unique_ptr <std::ofstream> output_file_ptr;
        if (output != "-") {
            boost::filesystem::path path = output;
            output_file_ptr.reset (new std::ofstream (path.native ()));
            if (!output_file_ptr->is_open ()) {
                std::ostringstream str;
                str << "Could not open " << path;
                throw std::runtime_error (str.str ());
            }
        }
        return output_file_ptr;
    }
}


int main (int argc, char *argv []) {
    int exit_code = EXIT_SUCCESS;

    (void) ::elf_version (EV_NONE);
    if (::elf_version (EV_CURRENT) == EV_NONE) {
        std::cerr << argv [0] << ": libelf.a out of date.\n";
        return EXIT_FAILURE;
    }

    try {
        boost::program_options::variables_map vm =
            get_program_options (argc, argv);

        if (vm.count ("version")) {
            // TODO: give the utility a proper version number!
            std::cout << argv [0] << " v1.0\n";
            return EXIT_SUCCESS;
        }

        assert (vm.count ("threads") == 1);
        unsigned num_threads = vm ["threads"].as <unsigned> ();
        assert (num_threads > 0);

        assert (vm.count ("output") == 1);

        auto const & input_files = vm ["input-file"];
        if (!input_files.empty ()) {
            output_flags ofl;
            ofl.verbose = vm ["verbose"].as <bool> ();
            ofl.quiet   = vm ["quiet"  ].as <bool> ();

            state_flags state;
            state.error = false;

            comdat_scanner scanner (ofl);
            auto file_paths = input_files.as <std::vector <std::string>> ();

            // Create the lock-free queue onto which we will push jobs.
            queue_type queue {file_paths.size ()};
            std::size_t const num_queued = queue_input_files (queue, file_paths, ofl);

            // Start the consumer threads. They'll immediately start pulling work from the
            // queue and then exit when it is exhausted.
            {
                updater progress (nullptr/*message*/,num_queued);
                if (!ofl.quiet) {
                    progress.run ();
                }

                boost::thread_group threads;
                while (num_threads-- > 0) {
                    threads.create_thread (boost::bind (consumer,
                                                        std::ref (queue), // the queue from which the consumer will read
                                                        &scanner,
                                                        std::cref (ofl), // output flags
                                                        &state,
                                                        std::ref (progress)));
                }
                // Wait for the worker threads to finish.
                threads.join_all ();
            }

            if (!state.error) {
                if (ofl.verbose) {
                    std::cout << "Dumping results\n";
                }

                auto output_file_ptr = output_file (vm ["output"].as <std::string> ());
                std::ostream & output_file = output_file_ptr.get () == nullptr
                                           ? std::cout
                                           : *output_file_ptr;

                output_file << scanner;
            }
            if (state.error) {
                exit_code = EXIT_FAILURE;
            }
        }
    } catch (std::exception const & ex) {
        std::cerr << "Exception: " << ex.what () << '\n';
        exit_code = EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown Exception\n";
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}
// eof main.cpp

