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

#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>

#include <boost/filesystem/path.hpp>

#include "dwarf_exception.hpp"
#include "options.hpp"
#include "process_file.hpp"
#include "worker_error.hpp"

int main (int argc, char const * argv[]) {
    int exit_code = EXIT_SUCCESS;
    try {
        auto const & options = program_options (argc, argv, std::cout);
        process_file (options);
    } catch (worker_error const & wex) {
        // No need to report anything: the worker thread will have done that already.
        exit_code = EXIT_FAILURE;
    } catch (help_exception const &) {
        // No need to report anything. The options parser wrote the help to the nominated stream.
        exit_code = EXIT_FAILURE;
    } catch (dwarf_exception const & dex) {
        std::cerr << "DWARF error: " << dex << '\n';
        exit_code = EXIT_FAILURE;
    } catch (std::exception const & ex) {
        std::cerr << "Error: " << ex.what () << '\n';
        exit_code = EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown exception\n";
        exit_code = EXIT_FAILURE;
    }

    return exit_code;
}
// eof main.cpp
