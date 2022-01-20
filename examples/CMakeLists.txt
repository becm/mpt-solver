# create MPT solver examples
cmake_minimum_required(VERSION 2.6)

add_compile_options("-DMPT_INCLUDE(x)=#x")

add_executable(vecpar vecpar.cpp)
target_include_directories(vecpar PRIVATE ${Mpt_CORE_INCLUDE_DIRS})
add_test(NAME vecpar COMMAND vecpar)
