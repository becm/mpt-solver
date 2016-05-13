# create mpt solver library
cmake_minimum_required(VERSION 2.6)

# solver library sources
file(GLOB sources
  client_*.c conf_*.c data_*.c init_*.c solver_*.c output_*.c
  steps_ode.c residuals_cdiff.c timeradd.c)

# use headers from install location
include_directories(${MPT_INSTALL_INCLUDE}/mpt)

# local version information
include_directories(${CMAKE_SOURCE_DIR}/base)

# library creation
add_library(mptsolver SHARED ${sources} libinfo.c)
set_target_properties(mptsolver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_link_libraries(mptsolver ${MPT_PLOT} ${MPT_LOADER} ${MPT_CORE})

install(TARGETS mptsolver DESTINATION ${MPT_INSTALL_LIB})
