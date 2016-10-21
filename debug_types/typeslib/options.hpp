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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <cassert>
#include <exception>
#include <iosfwd>
#include <memory>
#include <string>

#include <boost/optional.hpp>

class help_exception final : public std::exception {
public:
    help_exception ();
    ~help_exception () override;
    char const * what () const noexcept override;
};

struct options {
    options ();

    bool progress_enabled = true;
    unsigned threads;
    std::string input_path;
    boost::optional<std::string> count_output_path;
    boost::optional<std::string> contexts_output_path;


    class output_file_opener {
    public:
        explicit output_file_opener (boost::optional<std::string> const & path);
        ~output_file_opener ();

        // Moving.
        output_file_opener (output_file_opener && rhs);
        output_file_opener & operator= (output_file_opener && rhs);

        // No copying or assignment.
        output_file_opener (output_file_opener const &) = delete;
        output_file_opener & operator= (output_file_opener const &) = delete;

        operator bool () const noexcept {
            return os_ != nullptr;
        }

        std::ostream * get () noexcept {
            return os_;
        }
        std::ostream & operator* () noexcept {
            assert (os_ != nullptr);
            return *os_;
        }
        std::ostream * operator-> () noexcept {
            return os_;
        }

    private:
        std::unique_ptr<std::ofstream> file_;
        std::ostream * os_ = nullptr;
    };

    output_file_opener count_output_file () const;
    output_file_opener contexts_file () const;
};

options program_options (int argc, char const * argv[], std::ostream & help_stream);

#endif // OPTIONS_HPP
// eof options.hpp
