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
project (graph C CXX)

# ====================================
# configuring
# ====================================

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                       "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


# ====================================
# library
# ====================================

add_library (types STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/dt_config.h"
    append.hpp
    as_hex.hpp
    build_contexts.cpp
    build_contexts.hpp
    counts.cpp
    counts.hpp
    cu_iterator.cpp
    cu_iterator.hpp
    dwarf_exception.cpp
    dwarf_exception.hpp
    dwarf_helpers.cpp
    dwarf_helpers.hpp
    elf_helpers.cpp
    elf_helpers.hpp
    file_opener.cpp
    file_opener.hpp
    function_output_iterator.hpp
    iter_pair.hpp
    leb128.hpp
    md5.c
    md5.h
    names.cpp
    names.hpp
    options.cpp
    options.hpp
    process_file.cpp
    process_file.hpp
    scan_type.cpp
    scan_type.hpp
    sort_attributes.cpp
    sort_attributes.hpp
    visited_types.cpp
    visited_types.hpp
    worker_error.hpp
)
set_property (TARGET types PROPERTY CXX_STANDARD 11)
set_property (TARGET types PROPERTY CXX_STANDARD_REQUIRED Yes)
target_include_directories (types PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Set ARCH_IS_BIG_ENDIAN for md5.c
include (TestBigEndian)
test_big_endian (IS_BIG_ENDIAN)
target_compile_definitions (types PRIVATE -DARCH_IS_BIG_ENDIAN=${IS_BIG_ENDIAN})

if (MSVC)
    # Silence some of the microsoft compiler's less useful warnings.
    target_compile_definitions (types PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    target_compile_options (types PRIVATE /wd4996)

    # Appease VS2015 Update 2. It issues an error which says "You've instantiated 
    # std::atomic<T> with sizeof(T) equal to 2/4/8 and alignof(T) < sizeof(T)"
    target_compile_definitions (types PUBLIC -D_ENABLE_ATOMIC_ALIGNMENT_FIX)
endif ()



# ====================================
#
# Add the external dependencies
#



set (LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../shared_code/3rd_party")
if (TRUE) #MSVC)
    # On Windows, I build these things from source. I gave up trying to
    # make the external builds work :-(

    set (USE_LOCAL_BOOST YES)
    set (LOCAL_BOOST_PATH "${LIBS_DIR}/boost_1_60_0")

    set (USE_LOCAL_LIBELF YES)
    set (LOCAL_LIBELF_PATH "${LIBS_DIR}/libelf-0.8.13")

    set (USE_LOCAL_LIBDWARF YES)
    #set (LOCAL_LIBDWARF_PATH "${LIBS_DIR}/libdwarf-code")
    set (LOCAL_LIBDWARF_PATH "${LIBS_DIR}/dwarf-20160613")
elseif (TRUE) #APPLE)
    set (USE_LOCAL_LIBDWARF YES)
    #set (LOCAL_LIBDWARF_PATH "${LIBS_DIR}/libdwarf-code")
    set (LOCAL_LIBDWARF_PATH "${LIBS_DIR}/dwarf-20160613")
else ()
    message (STATUS "Boost/libelf/libdwarf must be pre-installed.")
endif ()


# ====================================
# ... Boost
# ====================================
if (USE_LOCAL_BOOST)
    add_subdirectory ("${LOCAL_BOOST_PATH}" libs/boost)
    target_link_libraries (types PUBLIC boost)
else ()
    find_package (Boost 1.54 REQUIRED COMPONENTS filesystem iostreams program_options system thread)
    target_include_directories (types SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries (types PUBLIC ${Boost_LIBRARIES})
endif ()


# ====================================
# ... libelf
# ====================================

if (USE_LOCAL_LIBELF)
    add_subdirectory ("${LOCAL_LIBELF_PATH}" libs/libelf)
else ()
    find_package (LibElf REQUIRED)
endif ()

target_compile_definitions (types PUBLIC ${LIBELF_DEFINITIONS})
target_include_directories (types SYSTEM PUBLIC ${LIBELF_INCLUDE_DIRS})
target_link_libraries (types PUBLIC ${LIBELF_LIBRARIES})


# ====================================
# ... libdwarf
# ====================================
if (USE_LOCAL_LIBDWARF)
    add_subdirectory ("${LOCAL_LIBDWARF_PATH}" libs/libdwarf)
    target_include_directories (types SYSTEM PUBLIC "${LOCAL_LIBDWARF_PATH}")
    target_link_libraries (types PUBLIC dwarf)
else ()
    find_package (LibDwarf REQUIRED)
    target_include_directories (types SYSTEM PUBLIC "${LIBDWARF_INCLUDE_DIRS}")
    target_compile_definitions (types PUBLIC ${LIBDWARF_DEFINITIONS})
    target_link_libraries (types PUBLIC "${LIBDWARF_LIBRARIES}")
endif ()


# ====================================
# ... threads
# ====================================
find_package (Threads)
target_link_libraries (types PUBLIC ${CMAKE_THREAD_LIBS_INIT})


# ====================================
# ... libelf
# ====================================
set (CMAKE_REQUIRED_INCLUDES ${LIBELF_INCLUDE_DIRS})
set (CMAKE_REQUIRED_DEFINITIONS ${LIBELF_DEFINITIONS})
include (CheckIncludeFileCXX)

check_include_file_cxx (libelf.h  HAVE_LIBELF_H)
check_include_file_cxx (libelf/libelf.h HAVE_LIBELF_LIBELF_H)
if (NOT HAVE_LIBELF_H AND NOT HAVE_LIBELF_LIBELF_H)
   message (SEND_ERROR "libelf.h was not found. Giving up.")
endif ()


# ====================================
# ... dt_config.h
# ====================================
message (STATUS "Writing: ${CMAKE_CURRENT_BINARY_DIR}/dt_config.h")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/config.in" "${CMAKE_CURRENT_BINARY_DIR}/dt_config.h")
target_include_directories (types PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")


#
# The local library containing code shared with other utilities
# in this package.
#

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/../../../shared_code/local" local)
target_link_libraries (types PUBLIC local)

#eof CMakeLists.txt
