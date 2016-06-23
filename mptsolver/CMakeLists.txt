# create mpt solver library
cmake_minimum_required(VERSION 2.6)

# solver library sources
file(GLOB sources
  client_*.c conf_*.c data_*.c init_*.c solver_*.c output_*.c
  steps_ode.c residuals_cdiff.c timeradd.c)

# library creation
add_library(mptsolver SHARED ${sources} libinfo.c)
set_target_properties(mptsolver PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_include_directories(mptsolver PRIVATE ${Mpt_INCLUDE_DIRS})
target_link_libraries(mptsolver
  ${Mpt_PLOT_LIBRARIES}
  ${Mpt_LOADER_LIBRARIES}
  ${Mpt_CORE_LIBRARIES}
)
install(TARGETS mptsolver COMPONENT solver DESTINATION ${MPT_INSTALL_LIB})
