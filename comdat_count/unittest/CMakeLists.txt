# Copyright (c) 2016 by SN Systems Ltd., Sony Interactive Entertainment Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

add_executable (unittest
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../shared_code/3rd_party/googletest/googlemock/src/gmock_main.cc"
    make_elf.cpp
    make_elf.hpp
    sections.cpp
    sections.h
    strings.cpp
    strings.h
    symbol_section.cpp
    symbol_section.h
    temporary_file.cpp
    temporary_file.h
    test_comdat_scanner.cpp
    test_digests.cpp
    test_elf_enumerator.cpp
    test_md5.cpp
    test_scanner.cpp
)

set_property (TARGET unittest PROPERTY CXX_STANDARD 11)
set_property (TARGET unittest PROPERTY CXX_STANDARD_REQUIRED Yes)

# Bump the warnings to maximum (or close to it)
if (MSVC)
    target_compile_options (unittest PRIVATE /W4)

    # Silence some of the microsoft compiler's less useful warnings.
    target_compile_definitions (unittest PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_options (unittest PRIVATE /wd4996)
elseif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options (unittest PRIVATE -Wall -Wextra -pedantic)
endif ()


target_include_directories (unittest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries (unittest PUBLIC local_test scanlib gtest gmock)

#eof CMakeLists.txt
