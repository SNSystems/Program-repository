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

add_executable (unit_test
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../shared_code/3rd_party/googletest/googlemock/src/gmock_main.cc"
    die_tree.hpp
    mock_debug.cpp
    mock_debug.hpp
    test_append.cpp
    test_build_contexts.cpp
    test_counts.cpp
    test_leb128.cpp
    test_as_hex.cpp
    test_cu_iterator.cpp
    test_iter_pair.cpp
    test_md5.cpp
    test_process_file.cpp
    test_scan_type.cpp
    test_sort_attributes.cpp
    test_visited_types.cpp
)

set_property (TARGET unit_test PROPERTY CXX_STANDARD 11)
set_property (TARGET unit_test PROPERTY CXX_STANDARD_REQUIRED Yes)

# Bump the warnings to maximum (or close to it)
if (MSVC)
    target_compile_options (unit_test PRIVATE /W4)

    # Silence some of the microsoft compiler's less useful warnings.
    target_compile_definitions (unit_test PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_options (unit_test PRIVATE /wd4996)
elseif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options (unit_test PRIVATE -Wall -Wextra -pedantic)
endif ()


target_include_directories (unit_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries (unit_test PUBLIC types local_test gtest gmock)

#eof CMakeLists.txt
