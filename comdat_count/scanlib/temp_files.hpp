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

#ifndef TEMP_FILES_H
#define TEMP_FILES_H (1)

#include <boost/filesystem.hpp>

class temp_directory_creator {
public:
    temp_directory_creator ()
            : path_ (unique_temp_dir ()) {}
    ~temp_directory_creator () {
        boost::filesystem::remove_all (path_);
    }

    // No copying or assignment.
    temp_directory_creator (temp_directory_creator const &) = delete;
    temp_directory_creator & operator= (temp_directory_creator const &) = delete;

    boost::filesystem::path const & path () const {
        return path_;
    }

private:
    static boost::filesystem::path unique_temp_dir ();
    boost::filesystem::path path_;
};


class path_cleanup {
public:
    path_cleanup (boost::filesystem::path const & path, bool is_temp);
    path_cleanup (path_cleanup const &) = delete;
    path_cleanup (path_cleanup && other);
    ~path_cleanup ();

    path_cleanup & operator= (path_cleanup && other);
    path_cleanup & operator= (path_cleanup const &) = delete;

    boost::filesystem::path path () const {
        return path_;
    }

private:
    void cleanup ();
    boost::filesystem::path path_;
    bool is_temp_;
};

boost::filesystem::path temporary_file_path ();

#endif // TEMP_FILES_H
// eof temp_files.h
