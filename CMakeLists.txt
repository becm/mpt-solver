# create mpt solver resources
cmake_minimum_required(VERSION 2.6)

project(mptsolver)

include(base/mpt-config.cmake)

# non-generated sources
file(GLOB sources "client_*.c" "util/*.c")

# additional header locations
include_directories(${CMAKE_CURRENT_BINARY_DIR} .)
include_directories(base base/mptcore base/mptplot base/mptclient)
link_directories(${MPT_INSTALL_LIB})

# header files
set(headers solver.h)

add_library(mptsolver SHARED ${sources} libinfo.c)
set_target_properties(mptsolver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptsolver mptio mptplot mptcore ${QT_LIBRARIES})

install(TARGETS mptsolver DESTINATION ${MPT_INSTALL_LIB})
install(FILES ${headers}  DESTINATION ${MPT_INSTALL_INCLUDE}/mpt)

set(CPACK_PACKAGE_NAME "mpt-solver")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MPT solver interface library")

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Marc Becker") #required

INCLUDE(CPack)
