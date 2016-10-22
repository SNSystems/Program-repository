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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <ostream>
#include <thread>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>


namespace {

    // Additional command line parser which interprets '@something' as a
    // option "config-file" with the value "something"
    std::pair<std::string, std::string> at_option_parser (std::string const & s) {
        if (s[0] == '@')
            return std::make_pair (std::string ("response-file"), s.substr (1));
        else
            return std::pair<std::string, std::string> ();
    }


    void check_threads (unsigned value) {
        if (value < 1) {
            std::ostringstream str;
            str << "Number of threads must be at least 1 (got " << value << ')';
            throw std::runtime_error (str.str ());
        }
    }
}


boost::program_options::variables_map get_program_options (int argc, char * argv[]) {
    namespace po = boost::program_options;

    // Declare a group of options that will be
    // allowed only on command line
    po::options_description generic ("Generic options");
    auto const default_threads = std::max (std::thread::hardware_concurrency (), 1U);
    generic.add_options () ("version", "print version string") ("help", "produce help message") (
        "quiet,q", po::bool_switch ()->default_value (false), "quiet output") (
        "threads,t",
        po::value<unsigned> ()->default_value (default_threads)->notifier (&check_threads),
        "the number of threads to use") ("verbose,v", po::bool_switch ()->default_value (false),
                                         "produce verbose output") (
        "response-file", po::value<std::string> (), "can be specified with '@name', too") (
        "output,o", po::value<std::string> ()->composing ()->default_value ("-"),
        "the file to which output will be written ('-' indicates stdout");

    // Declare a group of options that will be
    // allowed both on command line and in
    // config file
    po::options_description config ("Configuration");
    config.add_options ();

    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden ("Hidden options");
    hidden.add_options () ("input-file", po::value<std::vector<std::string>> ()->composing (),
                           "input file");


    po::options_description cmdline_options;
    cmdline_options.add (generic).add (hidden);

    po::options_description visible ("Allowed options");
    visible.add (generic);

    po::positional_options_description positional;
    positional.add ("input-file", -1);

    po::variables_map vm;
    store (po::command_line_parser (argc, argv)
               .options (cmdline_options)
               .positional (positional)
               .extra_parser (at_option_parser)
               .run (),
           vm);

    if (vm.count ("help")) {
        std::cout << visible << "\n";
        std::exit (EXIT_SUCCESS);
    }

    if (vm.count ("response-file")) {
        // Load the file and tokenize it
        std::ifstream ifs (vm["response-file"].as<std::string> ().c_str ());
        if (!ifs) {
            throw std::runtime_error ("Could not open the response file");
        }

        // Read the whole file into a string
        std::stringstream ss;
        ss << ifs.rdbuf ();

        // Split the file content
        boost::char_separator<char> sep (" \n\r");
        std::string sstr = ss.str ();
        boost::tokenizer<boost::char_separator<char>> tok (sstr, sep);
        std::vector<std::string> args;
        std::copy (tok.begin (), tok.end (), std::back_inserter (args));

        // Parse the file and store the options
        store (
            po::command_line_parser (args).options (cmdline_options).positional (positional).run (),
            vm);
    }

    notify (vm);
    return vm;
}

// eof options.cpp
