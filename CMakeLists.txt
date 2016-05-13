# create mpt solver resources
cmake_minimum_required(VERSION 2.6)

project(mptsolver)

# MPT config options
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/base")
find_package(Mpt REQUIRED)

# require libraries in target location
find_library(MPT_CORE NAMES mptcore PATHS "${MPT_INSTALL_LIB}" NO_DEFAULT_PATH DOC "mpt core library")
if (NOT MPT_CORE)
  add_subdirectory(base/mptcore)
  include_directories(base/mptcore)
  set(MPT_CORE mptcore)
endif()

find_library(MPT_PLOT NAMES mptplot PATHS "${MPT_INSTALL_LIB}" NO_DEFAULT_PATH DOC "mpt plotting library")
if (NOT MPT_PLOT)
  add_subdirectory(base/mptplot)
  include_directories(base/mptplot)
  set(MPT_PLOT mptplot)
endif()

find_library(MPT_LOADER NAMES mptloader PATHS "${MPT_INSTALL_LIB}" NO_DEFAULT_PATH DOC "mpt loader library")
if (NOT MPT_LOADER)
  add_subdirectory(base/mptloader)
  include_directories(base/mptloader)
  set(MPT_LOADER mptloader)
endif()

# primary header locations
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# header files
install(FILES solver.h DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)

# rules for solver library
add_subdirectory(mptsolver)

# package creation
set(CPACK_PACKAGE_NAME "mptsolver")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MPT solver interface library")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Becker") #required

INCLUDE(CPack)
