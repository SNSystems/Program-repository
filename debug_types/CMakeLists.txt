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
project (debug_types C CXX)

# ====================================
# configuring
# ====================================

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# ====================================
# executable
# ====================================

add_executable (debug_types main.cpp)
set_property (TARGET debug_types PROPERTY CXX_STANDARD 11)
set_property (TARGET debug_types PROPERTY CXX_STANDARD_REQUIRED Yes)

add_subdirectory (typeslib)
target_link_libraries (debug_types PUBLIC types)


# ====================================
# The Google Test/Mock frameworks
# ====================================

if (WIN32)
   # This Voodoo is required to build the project with MSVC
   set (gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll")
endif()
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/../../shared_code/3rd_party/googletest" googletest)


# ====================================
# tests
# ====================================

add_subdirectory (unit_test)
add_custom_command (TARGET debug_types
                    PRE_LINK
                    COMMAND unit_test "--gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/tests.xml"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                    COMMENT "Running unit tests"
                    VERBATIM)

#eof CMakeLists.txt
