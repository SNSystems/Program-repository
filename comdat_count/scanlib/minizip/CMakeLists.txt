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

project (q_minizip)

set (Q_MINIZIP_SRC
    ioapi.c
    ioapi.h
    unzip.c
    unzip.h
    zip.h
)

if (WIN32)
    list (APPEND Q_MINIZIP_SRC iowin32.c iowin32.h)
endif ()

add_library (q_minizip STATIC ${Q_MINIZIP_SRC})
target_include_directories (q_minizip PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions (q_minizip PUBLIC -DSTRICTUNZIP -DNOUNCRYPT)

if (TRUE) #MSVC)
   set (USE_LOCAL_ZLIB YES)
   set (LOCAL_ZLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../shared_code/3rd_party/zlib-1.2.8/")
endif ()

if (USE_LOCAL_ZLIB)
    set (CMAKE_MACOSX_RPATH ON)
    add_subdirectory ("${LOCAL_ZLIB_PATH}" libs/zlib)
    target_link_libraries (q_minizip PUBLIC zlibstatic)

    # TODO: I don't fully understand why this is necessary. Figure it out!
    get_property (dirs TARGET zlibstatic PROPERTY INCLUDE_DIRECTORIES)
    target_include_directories (q_minizip PUBLIC ${dirs})
else ()
    find_package (ZLIB REQUIRED)
    if (ZLIB_FOUND)
        target_include_directories (q_minizip PUBLIC ${ZLIB_INCLUDE_DIRS})
        target_link_libraries (q_minizip PUBLIC ${ZLIB_LIBRARIES})
    endif ()
endif ()
#eof
