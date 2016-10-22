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

#include "temp_files.hpp"

#include <memory>
#include <mutex>
#include <sstream>

boost::filesystem::path temp_directory_creator::unique_temp_dir () {
    // Find the nominated temporary directory, then create a (likely) unique
    // name inside it.
    boost::filesystem::path resl = boost::filesystem::temp_directory_path ();
    resl /= "%%%%-%%%%-%%%%-%%%%";
    resl = boost::filesystem::unique_path (resl);

    // Create that directory.
    boost::filesystem::create_directories (resl);
    return resl;
}


boost::filesystem::path temporary_file_path () {
    static std::mutex temp_dir_lock;
    static std::unique_ptr<temp_directory_creator> temp_dir;
    static auto uniquifier = 0U;

    std::lock_guard<std::mutex> guard (temp_dir_lock);
    if (!temp_dir) {
        temp_dir.reset (new temp_directory_creator ());
    }

    std::ostringstream temp_file_name;
    temp_file_name << "t" << uniquifier++;

    boost::filesystem::path temp_file = temp_dir->path ();
    temp_file /= temp_file_name.str ();

    return temp_file;
}



path_cleanup::path_cleanup (boost::filesystem::path const & path, bool is_temp)
        : path_ (path)
        , is_temp_ (is_temp) {}

path_cleanup::path_cleanup (path_cleanup && other)
        : path_ (other.path_)
        , is_temp_ (other.is_temp_) {

    other.is_temp_ = false;
}

path_cleanup & path_cleanup::operator= (path_cleanup && other) {
    if (this != &other) {
        this->cleanup ();
        path_ = other.path_;
        is_temp_ = other.is_temp_;
        other.is_temp_ = false;
    }
    return *this;
}

path_cleanup::~path_cleanup () {
    this->cleanup ();
}
void path_cleanup::cleanup () {
    if (is_temp_) {
        boost::filesystem::remove (path_);
        is_temp_ = false;
    }
}
