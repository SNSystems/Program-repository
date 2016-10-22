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

#include "elf_enumerator.hpp"

// Standard library includes
#include <list>
#include <ostream>
#include <system_error>
#include <tuple>

// 3rd party includes
#include <gmock/gmock.h>

// scanlib
#include "comdat_scanner.hpp"
#include "digests.hpp"
#include "elf_helpers.hpp"
#include "make_elf.hpp"

// Local includes
#include "temporary_file.h"

namespace {
    // ***********
    // * scanner *
    // ***********
    class scanner final : public comdat_scanner_base {
    public:
        struct member {
            boost::filesystem::path path;
            md5::digest digest;
        };
        typedef std::list<member> container;

        MOCK_METHOD2 (scan, void(boost::filesystem::path const & user_file_path, Elf * const elf));
        MOCK_METHOD2 (skip, void(boost::filesystem::path const & user_file_path, Elf * const elf));

        void record_scan (boost::filesystem::path const & user_file_path, Elf * const elf) {
            digests_.push_back ({user_file_path, digests::md5 (elf)});
        }
        container const & members () const {
            return digests_;
        }

    private:
        container digests_;
    };

    bool operator== (scanner::member const & lhs, scanner::member const & rhs) {
        return std::tie (lhs.path, lhs.digest) == std::tie (rhs.path, rhs.digest);
    }
    bool operator!= (scanner::member const & lhs, scanner::member const & rhs) {
        return !operator== (lhs, rhs);
    }


    // *******************
    // * ios_flags_saver *
    // *******************
    class ios_flags_saver {
    public:
        ios_flags_saver (std::ostream & os)
                : os_ (os)
                , flags_ (os.flags ()) {}
        ~ios_flags_saver () {
            os_.flags (flags_);
        }

    private:
        std::ostream & os_;
        std::ios::fmtflags flags_;
    };

    std::ostream & operator<< (std::ostream & os, scanner::member const & m) {
        ios_flags_saver saver (os);
        os << std::setfill ('0') << std::hex << "{ path: " << m.path << ", digest: ";
        for (unsigned c : m.digest) {
            os << std::setw (2) << c;
        }
        return os << " }";
    }


    // *****************
    // * ElfEnumerator *
    // *****************
    class ElfEnumerator : public ::testing::Test {
    protected:
        void SetUp () override {
            ASSERT_EQ (0, ::elf_errno ());
        }

        void TearDown () override {
            ASSERT_EQ (0, ::elf_errno ());
        }

        static std::size_t read (void * buffer, std::size_t bytes, file_ptr const & in);
        static void write (void const * buffer, std::size_t bytes, file_ptr const & out);
        static void seek (file_ptr const & f, long pos);
        static void seek_end (file_ptr const & f);
        static unsigned long file_size (file_ptr const & file);
        static void copy_file (file_ptr const & infile, file_ptr const & outfile);
    };

    // read
    // ~~~~
    std::size_t ElfEnumerator::read (void * buffer, std::size_t bytes, file_ptr const & in) {
        FILE * const inp = in.get ();
        std::size_t const bytes_read = std::fread (buffer, 1, bytes, inp);
        if (std::ferror (inp)) {
            throw std::system_error (errno, std::generic_category (), "fread() failed");
        }
        return bytes_read;
    }

    // write
    // ~~~~~
    void ElfEnumerator::write (void const * buffer, std::size_t bytes, file_ptr const & out) {
        FILE * const outp = out.get ();
        std::fwrite (buffer, 1, bytes, outp);
        if (std::ferror (outp)) {
            throw std::system_error (errno, std::generic_category (), "fwrite() failed");
        }
    }

    // seek
    // ~~~~
    void ElfEnumerator::seek (file_ptr const & f, long pos) {
        if (std::fseek (f.get (), pos, SEEK_SET) != 0) {
            throw std::system_error (errno, std::generic_category (), "fseek() failed");
        }
    }

    // seek_end
    // ~~~~~~~~
    void ElfEnumerator::seek_end (file_ptr const & f) {
        if (std::fseek (f.get (), 0L, SEEK_END) != 0) {
            throw std::system_error (errno, std::generic_category (), "fseek() failed");
        }
    }

    // file_size
    // ~~~~~~~~~
    unsigned long ElfEnumerator::file_size (file_ptr const & file) {
        seek_end (file);
        long size = std::ftell (file.get ());
        if (size == -1) {
            throw std::system_error (errno, std::generic_category (), "ftell() failed");
        }
        assert (size >= 0);
        return static_cast<unsigned long> (size);
    }

    // copy_file
    // ~~~~~~~~~
    void ElfEnumerator::copy_file (file_ptr const & infile, file_ptr const & outfile) {
        std::uint8_t buffer[256];
        std::size_t bytes = 0;

        seek (infile, 0);
        seek_end (outfile);
        while ((bytes = read (buffer, sizeof (buffer), infile)) > 0) {
            write (buffer, bytes, outfile);
        }
    }
}

TEST_F (ElfEnumerator, SimpleElf) {
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::ContainerEq;

    file_ptr file = temporary_file ();

    // Write an empty ELF.
    elf::update (::make_le32_elf (file.get ()), ELF_C_WRITE);

    boost::filesystem::path path = "user_file_path";
    scanner::container const expected{
        {
            path, digests::md5 (elf::begin (file.get (), ELF_C_READ)),
        },
    };

    // Hand the file to the enumerator.
    scanner sc;
    ON_CALL (sc, scan (_, _)).WillByDefault (Invoke (&sc, &scanner::record_scan));
    EXPECT_CALL (sc, scan (path, _)).Times (1);
    EXPECT_CALL (sc, skip (_, _)).Times (0);

    enumerate (fileno (file.get ()), path, &sc, nullptr);

    // Check that it found our file.
    scanner::container const & actual = sc.members ();
    EXPECT_THAT (actual, ContainerEq (expected));
}

TEST_F (ElfEnumerator, TextFileIsSkipped) {
    using ::testing::_;
    using ::testing::Invoke;

    file_ptr file = temporary_file ();
    char const buffer[] = "Hello, world\n";
    write (buffer, sizeof (buffer), file);

    boost::filesystem::path path = "user_file_path";

    // Hand the file to the enumerator.
    scanner sc;
    ON_CALL (sc, scan (_, _)).WillByDefault (Invoke (&sc, &scanner::record_scan));
    EXPECT_CALL (sc, scan (path, _)).Times (0);
    EXPECT_CALL (sc, skip (path, _)).Times (1);
    enumerate (fileno (file.get ()), path, &sc, nullptr);
}

TEST_F (ElfEnumerator, ArchiveContainingElf) {
    using ::testing::_;
    using ::testing::Invoke;

    file_ptr file = temporary_file ();
    file_ptr archive = temporary_file ();
    FILE * const archivep = archive.get ();

    // Write an empty ELF.
    elf::update (make_le64_elf (file.get ()), ELF_C_WRITE);

    boost::filesystem::path const path = "user_file_path";
    scanner::member const expected = {
        path, digests::md5 (elf::begin (file.get (),
                                        ELF_C_READ)), // Write an empty ELF and return its digest.
    };

    // Write the System V (GNU) variant archive header fields.
    std::fprintf (archivep, "!<arch>\x0a");
    std::fprintf (archivep, "%-16s", "foo/");
    std::fprintf (archivep, "%-12d", 0);                 // time stamp
    std::fprintf (archivep, "%-6d", 0);                  // owner
    std::fprintf (archivep, "%-6d", 0);                  // group
    std::fprintf (archivep, "%-8o", 0644);               // file mode
    std::fprintf (archivep, "%-10lu", file_size (file)); // file size
    std::fprintf (archivep, "\x60\x0a");                 // file magic
    // Now append the ELF file to the archive.
    copy_file (file, archive);

    seek (archive, 0L);

    // Hand the file to the enumerator.
    scanner sc;
    ON_CALL (sc, scan (_, _)).WillByDefault (Invoke (&sc, &scanner::record_scan));
    EXPECT_CALL (sc, scan (path, _)).Times (1);
    EXPECT_CALL (sc, skip (_, _)).Times (0);
    enumerate (fileno (archivep), path, &sc, nullptr);

    // Check that it found our file.
    EXPECT_THAT (sc.members (), scanner::container{expected});
}

TEST_F (ElfEnumerator, ArchiveContainingATextFile) {
    using ::testing::_;
    using ::testing::Invoke;

    file_ptr file = temporary_file ();
    file_ptr archive = temporary_file ();

    char const buffer[] = "Hello, world.";
    write (buffer, sizeof (buffer), file);

    FILE * const archivep = archive.get ();

    // Write the System V (GNU) variant archive header fields.
    std::fprintf (archivep, "!<arch>\x0a");
    std::fprintf (archivep, "%-16s", "foo/");
    std::fprintf (archivep, "%-12d", 0);                 // time stamp
    std::fprintf (archivep, "%-6d", 0);                  // owner
    std::fprintf (archivep, "%-6d", 0);                  // group
    std::fprintf (archivep, "%-8o", 0644);               // file mode
    std::fprintf (archivep, "%-10lu", file_size (file)); // file size
    std::fprintf (archivep, "\x60\x0a");                 // file magic
    // Now append the ELF file to the archive.
    copy_file (file, archive);

    seek (archive, 0L);

    boost::filesystem::path path = "user_file_path";

    // Hand the file to the enumerator.
    scanner sc;
    ON_CALL (sc, scan (_, _)).WillByDefault (Invoke (&sc, &scanner::record_scan));
    EXPECT_CALL (sc, scan (_, _)).Times (0);
    EXPECT_CALL (sc, skip (path, _)).Times (1);
    enumerate (fileno (archivep), path, &sc, nullptr);
}
// eof unittest/test_elf_enumerator.cpp
