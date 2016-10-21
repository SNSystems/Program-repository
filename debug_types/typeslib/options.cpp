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

#include "options.hpp"

#include <fstream>
#include <ostream>
#include <thread>

#include <boost/program_options.hpp>


//#include <vector>
#include <iostream>

// ******************
// * help_exception *
// ******************
// (ctor)
// ~~~~~~
help_exception::help_exception () {}

// (dtor)
// ~~~~~~
help_exception::~help_exception () {}

// what
// ~~~~
char const * help_exception::what () const noexcept {
    return "help exception";
}


// ***********
// * options *
// ***********
// (ctor)
// ~~~~~~
options::options ()
        : threads (std::max (std::thread::hardware_concurrency (), 1U)) {}

// count_output_file
// ~~~~~~~~~~~~~~~~~
auto options::count_output_file () const -> output_file_opener {
    return output_file_opener (count_output_path);
}

// contexts_file
// ~~~~~~~~~~~~~
auto options::contexts_file () const -> output_file_opener {
    return output_file_opener (contexts_output_path);
}


// *******************************
// * options::output_file_opener *
// *******************************
// (ctor)
// ~~~~~~
options::output_file_opener::output_file_opener (boost::optional<std::string> const & path)
        : file_ ()
        , os_ (nullptr) {

    if (path) {
        if (*path == "-") {
            os_ = &std::cout;
        } else {
            std::string p = *path;
            file_ = std::unique_ptr<std::ofstream> (new std::ofstream (p));
            os_ = file_.get ();
        }
    }
}

options::output_file_opener::output_file_opener (output_file_opener && rhs)
        : file_ (std::move (rhs.file_))
        , os_ (std::move (rhs.os_)) {}

// (dtor)
// ~~~~~~
options::output_file_opener::~output_file_opener () {}

// operator=
// ~~~~~~~~~
auto options::output_file_opener::operator= (output_file_opener && rhs)
    -> options::output_file_opener & {
    if (this != &rhs) {
        file_ = std::move (rhs.file_);
        os_ = std::move (rhs.os_);
    }
    return *this;
}


// *******************
// * program_options *
// *******************
options program_options (int argc, char const * argv[], std::ostream & help_stream) {
    options result;

    std::string count_output_path;
    std::string contexts_output_path;

    // Setup options
    // clang-format off
    namespace po = boost::program_options;
    po::options_description desc ("Options");
    desc.add_options ()
        ("help", "produce a help message")
        ("no-progress", po::bool_switch (), "disable the progress thermometer")
        ("threads,t", po::value<unsigned> (&result.threads)->default_value (result.threads), "the number of worker threads")
        ("count", po::value<std::string> (&count_output_path), "Count output JSON file ('-' for stdout)")
        ("contexts", po::value<std::string> (&contexts_output_path), "Contexts output JSON file ('-' for stdout)")
        ;

    po::options_description hidden;
    hidden.add_options ()
        ("input-path", po::value<std::string> (&result.input_path)->required (), "input path");
    // clang-format on

    po::positional_options_description positional;
    positional.add ("input-path", 1);

    // Parse the command line.
    po::variables_map vm;
    po::store (po::command_line_parser (argc, argv)
                   .options (po::options_description ().add (desc).add (hidden))
                   .positional (positional)
                   .run (),
               vm);

    if (vm.count ("help")) {
        help_stream << "Usage: " << argv[0] << " [options] input-file\n" << desc << '\n';
        throw help_exception ();
    }

    po::notify (vm);

    if (vm.count ("count")) {
        result.count_output_path = count_output_path;
    }
    if (vm.count ("contexts")) {
        result.contexts_output_path = contexts_output_path;
    }
    result.progress_enabled = !(vm["no-progress"].as<bool> ());
    assert (vm.count ("input-path") == 1);

    return result;
}

// eof options.cpp
