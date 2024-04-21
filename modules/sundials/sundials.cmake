# sundials.cmake
#   Suggested initial CMake setup for SUNDIALS libraries

# install to MPT base directory
set(CMAKE_INSTALL_PREFIX "$ENV{MPT_PREFIX}" CACHE PATH "Top level install directory")

# enable features
set(ENABLE_PTHREAD ON CACHE BOOL "Enable PTHREAD support")

# enable and set path for SUNDIALS examples
set(EXAMPLES_INSTALL ON CACHE BOOL "Enable installation of SUNDIALS examples")
set(EXAMPLES_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/examples" CACHE PATH "Target path for SUNDIALS examples")

# supply library locations for LAPACK/BLAS libraries
set(ENABLE_LAPACK ON CACHE BOOL "Enable LAPACK backend support")
set(LAPACK_LIBRARIES
	"${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/liblapack.so"
	"${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libblas.so"
	CACHE STRING "full path to LAPACK *and* BLAS library")
