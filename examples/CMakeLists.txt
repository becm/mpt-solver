# create MPT solver examples
cmake_minimum_required(VERSION 2.6)

add_compile_options("-DMPT_INCLUDE(x)=#x")

add_executable(vecpar vecpar.cpp)
target_include_directories(vecpar PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
add_test(NAME vecpar COMMAND vecpar)

add_executable(output output.cpp)
target_link_libraries(output ${Mpt_CXX_LIBRARIES})
target_include_directories(output PRIVATE
  ${Mpt_PLOT_INCLUDE_DIRS}
  ${Mpt_CORE_INCLUDE_DIRS}
)
add_test(NAME output COMMAND output)

add_executable(client client.cpp solver_run.c)
target_link_libraries(client
  mptsolver
  ${Mpt_IO_LIBRARIES}
  ${Mpt_CXX_LIBRARIES}
  ${Mpt_CORE_LIBRARIES}
)
target_include_directories(client PRIVATE
  ${Mpt_IO_INCLUDE_DIRS}
  ${Mpt_CORE_INCLUDE_DIRS}
)
add_test(NAME client COMMAND client)
