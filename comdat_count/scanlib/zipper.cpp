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

#include "zipper.hpp"
#include <fstream>
#include <sstream>
#include <system_error>

namespace {
    class file {
    public:
        explicit file (unzFile uf, boost::filesystem::path const & zip_path);
        ~file ();

    private:
        unzFile uf_;
    };

    file::file (unzFile uf, boost::filesystem::path const & zip_path)
            : uf_ (uf) {
        assert (uf != nullptr);
        int err = unzOpenCurrentFile (uf);
        if (err != UNZ_OK) {
            zipper::throw_unzip_error (err, zip_path);
        }
    }
    file::~file () {
        ::unzCloseCurrentFile (uf_);
        // (discard any error)
    }
}


namespace zipper {
    // -----------------
    // exception
    // -----------------

    // (ctor)
    // ~~~~~~
    exception::exception (int err, boost::filesystem::path const & zip_path)
            : std::runtime_error (message (err, zip_path)) {}

    exception::exception (char const * msg, boost::filesystem::path const & zip_path)
            : std::runtime_error (message (msg, zip_path)) {}
    exception::exception (std::string const & msg, boost::filesystem::path const & zip_path)
            : exception (msg.c_str (), zip_path) {}

    // message [static]
    // ~~~~~~~
    std::string exception::message (int err, boost::filesystem::path const & zip_path) {
        std::ostringstream str;
        str << "A ZIP error occurred: " << err << " (" << zip_path << ")";
        return str.str ();
    }
    std::string exception::message (char const * msg, boost::filesystem::path const & zip_path) {
        assert (msg != nullptr);
        std::ostringstream str;
        str << msg << ' ' << zip_path;
        return str.str ();
    }
}



namespace zipper {
    // -----------------
    // throw unzip error
    // -----------------
    void throw_unzip_error (int err, boost::filesystem::path const & zip_path) {
        if (err != UNZ_OK) {
            if (err == UNZ_ERRNO) {
                throw std::system_error (errno, std::generic_category ());
            } else {
                throw exception (err, zip_path);
            }
        }
    }

    void throw_unzip_error (char const * msg, boost::filesystem::path const & zip_path) {
        assert (msg != nullptr);
        throw exception (msg, zip_path);
    }

    void throw_unzip_error (std::string const & msg, boost::filesystem::path const & zip_path) {
        throw_unzip_error (msg.c_str (), zip_path);
    }

    // ----
    // open
    // ----
    zip_ptr open (boost::filesystem::path const & path) {
        zip_ptr uf{::unzOpen64 (path.string ().c_str ()), &::unzClose};
        if (uf.get () == nullptr) {
            throw exception ("Could not open", path);
        }
        return uf;
    }
    zip_ptr open (boost::filesystem::path const & path, std::nothrow_t const &) {
        return zip_ptr{::unzOpen64 (path.string ().c_str ()), &::unzClose};
    }


    // -------
    // extract
    // -------
    void extract (unzFile uf, char const * member_name, boost::filesystem::path const & dest,
                  boost::filesystem::path const & zip_path) {

        assert (member_name != nullptr);

        std::ofstream out (dest.native (), std::ios::binary | std::ios::out | std::ios::trunc);
        if (!out.is_open ()) {
            std::ostringstream str;
            str << "Could not open " << dest;
            throw std::runtime_error (str.str ());
        }
        // The library will now throw if we can't write to the file.
        out.exceptions (std::ofstream::failbit | std::ofstream::badbit);

        int err = unzLocateFile (uf, member_name, 1 /* case sensitive compare */);
        if (err != UNZ_OK) {
            throw_unzip_error (err, zip_path);
        }

        static constexpr unsigned buffer_size = 65535;
        std::vector<char> buf (buffer_size);
        file current_file (uf, zip_path);
        for (;;) {
            int copied = unzReadCurrentFile (uf, buf.data (), buffer_size);
            if (copied == 0) {
                break;
            } else if (copied < 0) {
                throw_unzip_error (err, zip_path);
            }

            out.write (buf.data (), copied);
        }
    }

} // namespace zipper

// eof scanlib/zipper.cpp
