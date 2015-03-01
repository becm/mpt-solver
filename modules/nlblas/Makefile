# Makefile: nonlinear Solvers with BLAS dependancy
LIB ?= mpt_nlblas
#
# placeholder to avoid error
HEADER =
#
# shared library version
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
#
# linker settings
LDLIBS = -lblas
LINK   = ${FC}
#
# objects
include imsl/imsl.mk

MATH = ${imsl_objects}
OBJS = $(imsl_wrapper:%=imsl/%)

ADD_OBJECTS = $(nls_objects:%=../%) $(gen_objects:%=../%)
#
# general solver build settings
include ../mpt.solver.mk
