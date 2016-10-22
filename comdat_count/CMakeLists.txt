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

cmake_minimum_required (VERSION 3.0)
project (scan)


# ====================================
# configuring
# ====================================

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# ====================================
# find the local dependencies
# ====================================

add_subdirectory (scanlib)


# ====================================
# unit tests
# ====================================

set (gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll")
add_subdirectory (../../shared_code/3rd_party/googletest googletest)
add_subdirectory (unittest)


# ====================================
# executable
# ====================================

add_executable (scan main.cpp)
set_property (TARGET scan PROPERTY CXX_STANDARD 11)
set_property (TARGET scan PROPERTY CXX_STANDARD_REQUIRED Yes)
target_link_libraries (scan PRIVATE scanlib)

add_custom_command (TARGET scan
                    PRE_LINK
                    COMMAND unittest "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/tests.xml"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    COMMENT "Running unit tests"
                    VERBATIM)

#eof CMakeLists.txt
