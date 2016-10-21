# - Try to find libelf
# Once done this will define
#
#  LIBELF_FOUND - system has libelf
#  LIBELF_INCLUDE_DIRS - the libelf include directory
#  LIBELF_LIBRARIES - Link these to use libelf
#  LIBELF_DEFINITIONS - Compiler switches required for using libelf
#
#  Copyright (c) 2008 Bernhard Walle <bernhard.walle@gmx.de>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (LIBELF_LIBRARIES AND LIBELF_INCLUDE_DIRS)
  set (LibElf_FIND_QUIETLY TRUE)
endif (LIBELF_LIBRARIES AND LIBELF_INCLUDE_DIRS)

# First try for <libelf.h>
find_path (LIBELF_INCLUDE_DIRS
    NAMES libelf.h
    PATHS /usr/include
          /usr/local/include
          /opt/local/include
          /sw/include
    ENV CPATH)

# If that wasn't successful, look for <libelf/libelf.h>
find_path (LIBELF_INCLUDE_DIRS
    NAMES libelf/libelf.h
    PATHS /usr/include
          /usr/local/include
          /opt/local/include
          /sw/include
    ENV CPATH)


find_library (LIBELF_LIBRARIES
    NAMES elf
    PATHS /usr/lib
          /usr/local/lib
          /opt/local/lib
          /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBELF_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibElf DEFAULT_MSG
    LIBELF_LIBRARIES
    LIBELF_INCLUDE_DIRS)


mark_as_advanced(LIBELF_INCLUDE_DIRS LIBELF_LIBRARIES)
