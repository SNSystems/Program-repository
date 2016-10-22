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

project (scanlib)

# ====================================
# configuring
# ====================================

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")


# ====================================
# static library
# ====================================

add_library (scanlib
    consumer.cpp
    consumer.hpp
    comdat_scanner.cpp
    comdat_scanner.hpp
    digests.cpp
    digests.hpp
    elf_enumerator.cpp
    elf_enumerator.hpp
    elf_helpers.cpp
    elf_helpers.hpp
    elf_scanner.cpp
    elf_scanner.hpp
    flags.hpp
    options.cpp
    options.hpp
    producer.cpp
    producer.hpp
    temp_files.cpp
    temp_files.hpp
    zipper.cpp
    zipper.hpp
)
target_include_directories (scanlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


# ====================================
# compiler switches
# ====================================

# The library is written in C++11.
set_property (TARGET scanlib PROPERTY CXX_STANDARD 11)
set_property (TARGET scanlib PROPERTY CXX_STANDARD_REQUIRED Yes)

# Bump the warnings to maximum (or close to it)
if (MSVC)
    target_compile_options (scanlib PRIVATE /W4)

    # Silence some of the microsoft compiler's less useful warnings.
    target_compile_definitions (scanlib PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    # std::atomic fix in VS2015 Update 2.
    target_compile_definitions (scanlib PUBLIC -D_ENABLE_ATOMIC_ALIGNMENT_FIX)
    target_compile_options (scanlib PRIVATE /wd4996)
elseif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options (scanlib PRIVATE -Wall -Wextra -pedantic)
endif ()


# ====================================
#
# Add the external dependencies
#

# On Windows, I build these things from source. I gave up trying to
# make the external builds work :-(
if (TRUE)#MSVC)
    set (LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../shared_code/3rd_party")
   
    set (USE_LOCAL_BOOST YES)
    set (LOCAL_BOOST_PATH "${LIBS_DIR}/boost_1_60_0")

    set (USE_LOCAL_ZLIB YES)
    set (USE_LOCAL_LIBELF YES)
    set (LOCAL_LIBELF_PATH "${LIBS_DIR}/libelf-0.8.13")
endif ()


# ====================================
# find the local dependencies
# ====================================

add_subdirectory (minizip)
target_link_libraries (scanlib PUBLIC q_minizip)

add_subdirectory (md5)
target_link_libraries (scanlib PUBLIC md5)




# ====================================
# ... Boost
# ====================================
if (USE_LOCAL_BOOST)
    add_subdirectory ("${LOCAL_BOOST_PATH}" libs/boost)
    target_link_libraries (scanlib PUBLIC boost)
else ()
    find_package (Boost 1.54 REQUIRED
                  COMPONENTS filesystem iostreams program_options system thread)
    target_include_directories (scanlib SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries (scanlib PUBLIC ${Boost_LIBRARIES})
endif ()


# ====================================
# ... libelf
# ====================================
if (USE_LOCAL_LIBELF)
    add_subdirectory ("${LOCAL_LIBELF_PATH}" libs/libelf)
    target_link_libraries (scanlib PUBLIC elf)
else ()
    find_package (LibElf REQUIRED)
    target_compile_definitions (scanlib PUBLIC ${LIBELF_DEFINITIONS})
    target_include_directories (scanlib SYSTEM PUBLIC ${LIBELF_INCLUDE_DIRS})
    target_link_libraries (scanlib PUBLIC ${LIBELF_LIBRARIES})
endif ()


# ====================================
# ... threads
# ====================================
find_package (Threads)
target_link_libraries (scanlib PUBLIC ${CMAKE_THREAD_LIBS_INIT})


#
# The local library containing code shared with other utilities
# in this package.
#

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/../../../shared_code/local" local)
target_link_libraries (scanlib PUBLIC local)

#eof CMakeLists.txt
