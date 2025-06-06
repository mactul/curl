#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# SPDX-License-Identifier: curl
#
###########################################################################
# Find the quiceh library
#
# Input variables:
#
# - `QUICEH_INCLUDE_DIR`:   The quiceh include directory.
# - `QUICEH_LIBRARY`:       Path to `quiceh` library.
#
# Result variables:
#
# - `QUICEH_FOUND`:         System has quiceh.
# - `QUICEH_INCLUDE_DIRS`:  The quiceh include directories.
# - `QUICEH_LIBRARIES`:     The quiceh library names.
# - `QUICEH_LIBRARY_DIRS`:  The quiceh library directories.
# - `QUICEH_PC_REQUIRES`:   The quiceh pkg-config packages.
# - `QUICEH_CFLAGS`:        Required compiler flags.
# - `QUICEH_VERSION`:       Version of quiceh.

set(QUICEH_PC_REQUIRES "quiceh")

if(CURL_USE_PKGCONFIG AND
   NOT DEFINED QUICEH_INCLUDE_DIR AND
   NOT DEFINED QUICEH_LIBRARY)
  find_package(PkgConfig QUIET)
  pkg_check_modules(QUICEH ${QUICEH_PC_REQUIRES})
endif()

if(QUICEH_FOUND)
  set(Quiche_FOUND TRUE)
  string(REPLACE ";" " " QUICEH_CFLAGS "${QUICEH_CFLAGS}")
  message(STATUS "Found Quiche (via pkg-config): ${QUICEH_INCLUDE_DIRS} (found version \"${QUICEH_VERSION}\")")
else()
  find_path(QUICEH_INCLUDE_DIR NAMES "quiceh.h")
  find_library(QUICEH_LIBRARY NAMES "quiceh")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Quiche
    REQUIRED_VARS
      QUICEH_INCLUDE_DIR
      QUICEH_LIBRARY
  )

  if(QUICEH_FOUND)
    set(QUICEH_INCLUDE_DIRS ${QUICEH_INCLUDE_DIR})
    set(QUICEH_LIBRARIES    ${QUICEH_LIBRARY})
  endif()

  mark_as_advanced(QUICEH_INCLUDE_DIR QUICEH_LIBRARY)
endif()
