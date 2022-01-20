# create mpt solver resources
cmake_minimum_required(VERSION 2.6)

project(mptsolver)

# MPT config options
list(APPEND CMAKE_PREFIX_PATH "base")
find_package(Mpt COMPONENTS Cxx Plot Loader IO Core)

# primary header locations
include_directories(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})

# header files
install(FILES solver.h COMPONENT headers DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)

# rules for solver library
add_subdirectory(mptsolver)

# enable tests
enable_testing()
add_subdirectory(examples)

# package creation
set(CPACK_PACKAGE_NAME "mptsolver")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MPT solver interface library")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Becker") #required

INCLUDE(CPack)
