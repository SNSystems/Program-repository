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

#ifndef SCANLIB_ZIPPER_HPP
#define SCANLIB_ZIPPER_HPP

#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "unzip.h"
#include <boost/filesystem.hpp>

namespace zipper {

    class exception : public std::runtime_error {
    public:
        exception (int err, boost::filesystem::path const & zip_path);
        exception (char const * msg, boost::filesystem::path const & zip_path);
        exception (std::string const & msg, boost::filesystem::path const & zip_path);

    private:
        static std::string message (int err, boost::filesystem::path const & zip_path);
        static std::string message (char const * msg, boost::filesystem::path const & zip_path);
    };

    using zip_ptr = std::unique_ptr<std::remove_pointer<unzFile>::type, decltype (&::unzClose)>;
    zip_ptr open (boost::filesystem::path const & path);
    zip_ptr open (boost::filesystem::path const & path, std::nothrow_t const &);


    void throw_unzip_error (int err, boost::filesystem::path const & zip_path);
    void throw_unzip_error (char const * msg, boost::filesystem::path const & zip_path);
    void throw_unzip_error (std::string const & msg, boost::filesystem::path const & zip_path);

    void extract (unzFile uf, char const * member_name, boost::filesystem::path const & dest,
                  boost::filesystem::path const & zip_path);

} // namespace zipper

#endif // SCANLIB_ZIPPER_HPP
// eof scanlib/zipper.hpp
