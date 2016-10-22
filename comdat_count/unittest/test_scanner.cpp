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

#include "elf_scanner.hpp"

// Standard library includes
#include <array>
#include <cstring>
#include <limits>
#include <type_traits>

// 3rd party includes
#include <gelf.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// scanlib includes
#include "elf_helpers.hpp"
#include "elf_scanner.hpp"

// local includes
#include "make_elf.hpp"
#include "sections.h"
#include "strings.h"
#include "symbol_section.h"
#include "temporary_file.h"

namespace {
    struct callback {
        virtual ~callback () {}
        void operator() (std::string const & name, std::uint64_t size) const {
            this->call (name, size);
        }
        virtual void call (std::string const & name, std::uint64_t size) const = 0;
    };

    struct mock_callback : public callback {
        MOCK_CONST_METHOD2 (call, void(std::string const &, std::uint64_t));
    };
}



TEST (Scanner, Be32NoGroups) {
    using ::testing::_;
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    std::array<std::uint32_t, 3> const data{{0x01234567, 0x89abcdef, 0xdeadc0de}};

    {
        elf::elf_ptr elf = make_elf <endian::big, 32U> (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        create_progbits_alloc_section (elf.get (), &data, section_names.append (".data"));
        symbols.commit (&section_names);
        create_section_names_section (elf.get (), &section_names);
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        // I don't expect the callback to be invoked.
        mock_callback cb;
        EXPECT_CALL (cb, call (_, _)).Times (0);

        // Unfortunately, Google Mock doesn't allow mocks to be copied (into an instance of
        // std::function in this case), so we need to bounce through a small lambda to call it.
        auto trampoline = [&cb](std::string const & name, std::uint64_t size) { cb (name, size); };
        scanner.scan (trampoline);
    }
}


TEST (Scanner, Le64NoGroups) {
    using ::testing::_;
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    std::array<std::uint32_t, 3> const data{{0x01234567, 0x89abcdef, 0xdeadc0de}};

    {
        elf::elf_ptr elf = make_elf <endian::little, 64U> (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        create_progbits_alloc_section (elf.get (), &data, section_names.append (".data"));
        symbols.commit (&section_names);
        create_section_names_section (elf.get (), &section_names);
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        // I don't expect the callback to be invoked.
        mock_callback cb;
        EXPECT_CALL (cb, call (_, _)).Times (0);
        scanner.scan ([&cb](std::string const & name, std::uint64_t size) { cb (name, size); });
    }
}


TEST (Scanner, Be32SingleComdatSectionWithOneMember) {
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    std::array<std::uint32_t, 3> const data{{0x01234567, 0x89abcdef, 0xdeadc0de}};

    {
        elf::elf_ptr elf = make_be32_elf (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        std::array<std::uint32_t, 2> group_data{{
            static_cast<std::uint32_t> (GRP_COMDAT),
            static_cast<std::uint32_t> (elf_ndxscn (
                create_progbits_alloc_section (elf.get (), &data, section_names.append (".data")))),
        }};
        create_group_section (elf.get (),
                              "identifier",                    // identifier symbol name
                              &symbols,                        // file symbol table
                              section_names.append (".group"), // section name
                              &group_data);

        // Add the symbol table, the section header string table, and finally the
        // file itself.
        symbols.commit (&section_names);

        // Now the section name string table.
        create_section_names_section (elf.get (), &section_names);

        // Write the final file.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        // Expect a single invocation of the callback with the correct symbol name and size equal
        // to that of the ".data" section.
        mock_callback cb;
        EXPECT_CALL (cb, call ("identifier", sizeof (decltype (data)::value_type) * data.size ()))
            .Times (1);
        scanner.scan ([&cb](std::string const & name, std::uint64_t size) { cb (name, size); });
    }
}


TEST (Scanner, Le64SingleComdatSectionWithOneMember) {
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    std::array<std::uint32_t, 3> const data{{0x01234567, 0x89abcdef, 0xdeadc0de}};

    {
        elf::elf_ptr elf = make_le64_elf (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        std::array<std::uint32_t, 2> group_data{{
            static_cast<std::uint32_t> (GRP_COMDAT),
            static_cast<std::uint32_t> (elf_ndxscn (
                create_progbits_alloc_section (elf.get (), &data, section_names.append (".data")))),
        }};
        create_group_section (elf.get (),
                              "identifier",                    // identifier symbol name
                              &symbols,                        // file symbol table
                              section_names.append (".group"), // section name
                              &group_data);

        // Add the symbol table, the section header string table, and finally the
        // file itself.
        symbols.commit (&section_names);

        // Now the section name string table.
        create_section_names_section (elf.get (), &section_names);

        // Write the final file.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        // Expect a single invocation of the callback with the correct symbol name and size equal
        // to that of the ".data" section.
        mock_callback cb;
        EXPECT_CALL (cb, call ("identifier", sizeof (decltype (data)::value_type) * data.size ()))
            .Times (1);
        scanner.scan ([&cb](std::string const & name, std::uint64_t size) { cb (name, size); });
    }
}


TEST (Scanner, Le64SingleNonComdatGroup) {
    using ::testing::_;
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    {
        elf::elf_ptr elf = make_le64_elf (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        std::array<std::uint32_t, 1> group_data{{
            static_cast<std::uint32_t> (~GRP_COMDAT),
        }};
        create_group_section (elf.get (),
                              "identifier",                    // identifier symbol name
                              &symbols,                        // file symbol table
                              section_names.append (".group"), // section name
                              &group_data);

        // Add the symbol table, the section header string table, and finally the
        // file itself.
        symbols.commit (&section_names);

        // Now the section name string table.
        create_section_names_section (elf.get (), &section_names);

        // Write the final file.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        // I don't expect the callback to be invoked.
        mock_callback cb;
        EXPECT_CALL (cb, call (_, _)).Times (0);
        scanner.scan ([&cb](std::string const & name, std::uint64_t size) { cb (name, size); });
    }
}


TEST (Scanner, Le64SingleCorruptComdatGroup) {
    using ::testing::_;
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    {
        elf::elf_ptr elf = make_le64_elf (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        std::array<std::uint32_t, 2> group_data{{
            static_cast<std::uint32_t> (GRP_COMDAT), std::numeric_limits<std::uint32_t>::max (),
        }};
        create_group_section (elf.get (),
                              "identifier",                    // identifier symbol name
                              &symbols,                        // file symbol table
                              section_names.append (".group"), // section name
                              &group_data);

        // Add the symbol table, the section header string table, and finally the
        // file itself.
        symbols.commit (&section_names);

        // Now the section name string table.
        create_section_names_section (elf.get (), &section_names);

        // Write the final file.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());
        ASSERT_THROW (scanner.scan ([](std::string const & /*name*/, std::uint64_t /*size*/) {}),
                      elf::exception);
    }
}


#if 0
// Test disabled: some versions of libelf won't allow the creation of an ELF
// with invalid group section length
TEST (Scanner, Be64SingleCorruptComdatGroupBadLength) {
    using ::testing::_;
    temporary_file file;

    {
        elf::elf_ptr elf = make_le64_elf (file.fd ());
        strings section_names;
        symbol_section symbols (elf.get ());

        // An array of 7 bytes (7 is not a multiple of 4...)
        std::array <std::uint8_t, 7> group_data {{
            (static_cast <std::uint32_t> (GRP_COMDAT) & 0xFF000000) >> 24,
            (static_cast <std::uint32_t> (GRP_COMDAT) & 0x00FF0000) >> 16,
            (static_cast <std::uint32_t> (GRP_COMDAT) & 0x0000FF00) >>  8,
            (static_cast <std::uint32_t> (GRP_COMDAT) & 0x000000FF) >>  0,
            std::uint32_t {0},
            std::uint32_t {0},
            std::uint32_t {0},
        }};

        Elf_Scn * const section = elf::newscn (elf.get ());
        // Populate it with data.
        add_section_data (section, &group_data);

        // Add the identifying symbol.
        std::size_t const symbol_index = symbols.add ("identifier", section, 0 /*offset*/, 0 /*size*/);

        // Properly configure the section header.
        GElf_Shdr shdr = gelf::getshdr (section);
        shdr.sh_name = section_names.append (".group");
        shdr.sh_type = SHT_GROUP;
        shdr.sh_flags = 0;
        shdr.sh_info = symbol_index; // symbol table index of the identifying entry.
        shdr.sh_entsize = sizeof (std::uint32_t);
        shdr.sh_link = elf_ndxscn (symbols.section ()); // identifying entry's symbol table.
        gelf::update_shdr (section, shdr);


        // Add the symbol table, the section header string table, and finally the
        // file itself.
        symbols.commit (&section_names);

        // Now the section name string table.
        create_section_names_section (elf.get (), &section_names);

        // Write the final file.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (file.fd (), ELF_C_READ);
        elf_scanner scanner (elf.get ());
        ASSERT_ANY_THROW (scanner.scan ([](std::string const & name, std::uint64_t size) {}));
    }
}
#endif


TEST (Scanner, SingleComdatSectionWithTwoMembers) {
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    std::array<std::uint32_t, 3> const data1{{0xcafebabe, 0xb01dface, 0xdeadc0de}};
    std::array<std::uint8_t, 3> const data2{{3, 5, 7}};

    std::size_t const expected_size = sizeof (decltype (data1)::value_type) * data1.size () +
                                      sizeof (decltype (data2)::value_type) * data2.size ();

    // Build the test ELF.
    {
        elf::elf_ptr elf = make_be32_elf (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        // Create the data section body, add symbols for it, and create the section header
        // table entry.
        std::array<std::uint32_t, 3> group_data{{
            static_cast<std::uint32_t> (GRP_COMDAT),
            static_cast<std::uint32_t> (elf_ndxscn (create_progbits_alloc_section (
                elf.get (), &data1, section_names.append (".data1")))),
            static_cast<std::uint32_t> (elf_ndxscn (create_progbits_alloc_section (
                elf.get (), &data2, section_names.append (".data2")))),
        }};
        create_group_section (elf.get (),
                              "ident",                         // identifier symbol name
                              &symbols,                        // file symbol table
                              section_names.append (".group"), // section name
                              &group_data);

        // Add the symbol table and section header names.
        symbols.commit (&section_names);
        create_section_names_section (elf.get (), &section_names);

        // Write the final file.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    // Now the test body
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        // Expect a single invocation of the callback with the correct symbol name and size equal
        // to that of the sum of the sizes of the two member sections.
        mock_callback cb;
        EXPECT_CALL (cb, call ("ident", expected_size)).Times (1);
        scanner.scan ([&cb](std::string const & name, std::uint64_t size) { cb (name, size); });
    }
}


TEST (Scanner, TwoComdatSectionsWithOneMemberEach) {
    file_ptr file = temporary_file ();
    int const fd = fileno (file.get ());

    std::array<std::uint32_t, 2> const data1{{0xcafebabe, 0xb01dface}};
    std::array<std::uint8_t, 3> const data2{{3, 5, 7}};

    // Build the test ELF.
    {
        elf::elf_ptr elf = make_be32_elf (fd);
        strings section_names;
        symbol_section symbols (elf.get ());

        // Create a group ("ident1") which contains a section whose contents are 'data1'.
        std::array<std::uint32_t, 2> group1_data{{
            static_cast<std::uint32_t> (GRP_COMDAT),
            static_cast<std::uint32_t> (elf_ndxscn (create_progbits_alloc_section (
                elf.get (), &data1, section_names.append (".data1")))),
        }};
        create_group_section (elf.get (),
                              "ident1",                         // identifier symbol name
                              &symbols,                         // file symbol table
                              section_names.append (".group1"), // section name
                              &group1_data);                    // content

        // Create a group ("ident2") which contains a section whose contents are 'data2'.
        std::array<std::uint32_t, 2> group2_data{{
            static_cast<std::uint32_t> (GRP_COMDAT),
            static_cast<std::uint32_t> (elf_ndxscn (create_progbits_alloc_section (
                elf.get (), &data2, section_names.append (".data2")))),
        }};
        create_group_section (elf.get (),
                              "ident2",                         // identifier symbol name
                              &symbols,                         // file symbol table
                              section_names.append (".group2"), // section name
                              &group2_data);

        // Add the symbol table and section header names.
        symbols.commit (&section_names);
        create_section_names_section (elf.get (), &section_names);

        // Write the final ELF.
        elf::update (elf.get (), ELF_C_WRITE);
    }
    {
        elf::elf_ptr elf = elf::begin (fd, ELF_C_READ);
        elf_scanner scanner (elf.get ());

        mock_callback cb;
        EXPECT_CALL (cb, call ("ident1", sizeof (decltype (data1)::value_type) * data1.size ()))
            .Times (1);
        EXPECT_CALL (cb, call ("ident2", sizeof (decltype (data2)::value_type) * data2.size ()))
            .Times (1);
        scanner.scan ([&cb](std::string const & name, std::uint64_t size) { cb (name, size); });
    }
}

// eof test_scanner.cpp
