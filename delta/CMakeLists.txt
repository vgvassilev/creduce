# Copyright (c) 2012, 2013, 2014, 2015 The University of Utah
# All rights reserved.
#
# This file is distributed under the University of Illinois Open Source
# License.  See the file COPYING for details.

cmake_minimum_required(VERSION 2.8.12)
find_package(FLEX)

project(topformflat)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})
add_definitions(-DHAVE_CONFIG_H)

FLEX_TARGET(topformflat_scanner topformflat.l ${PROJECT_BINARY_DIR}/topformflat.c)

add_executable(topformflat
  ${FLEX_topformflat_scanner_OUTPUTS}
)

install(TARGETS topformflat
  RUNTIME DESTINATION "bin"
)
