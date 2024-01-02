# Makefile: SUNDIALS interface library
LIB ?= mpt_sundials
#
# base location of SUNDIALS artifacts
sundials_version = 6.7.0
#
# include directories (generated, public)
INC = . '${SUNDIALS_BUILD}/include' '${SUNDIALS}/include'
#
# SUNDIALS modules to include in shared library
sundials_direct = band dense
sundials_lapack = lapackband lapackdense
sundials_iterative = spgmr spbcgs sptfqmr
sundials_linear = ${sundials_direct} ${sundials_lapack} ${sundials_iterative}

sundials_nonlinear = fixedpoint newton

sundials_matrix = band dense

sundials_solvers = $(sundials_linear:%=sunlinsol_%) $(sundials_nonlinear:%=sunnonlinsol_%)
sundials_auxiliary = ${sundials_matrix:%=sunmatrix_%} nvector_serial generic

sundials_modules = cvode ida kinsol ${sundials_solvers} ${sundials_auxiliary}
#
# shared library version
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
# linker settings
ifneq (${sundials_lapack},)
  DEF += MPT_WITH_LAPACK
  LDLIBS += -llapack -lblas
endif
LDLIBS += -lm
#
# shared components
mod_require = ${mod_ivp} ufcn_dae ufcn_ode
#
# module components
sources_cvode = $(wildcard cvode/*.c)
sources_ida = $(wildcard ida/*.c)
sources = $(wildcard sundials_*.c)

SRCS := ${sources} ${sources_cvode} ${sources_ida}

#
# headers to export
HEADER = sundials.h
CONFIG = sundials.conf
#
# general library rules
include ../mpt.solver.mk
#
# SUNDIALS source location
SUNDIALS ?= ${MATH_BASE}/sundials-${sundials_version}
#
# SUNDIALS precompiled object location
SUNDIALS_BUILD ?= ${SUNDIALS}/build
MATH_OBJS_STATIC != env LIBRARY_TYPE=static ./sundials_collect.sh "${SUNDIALS_BUILD}" ${sundials_modules}
MATH_OBJS_SHARED != env LIBRARY_TYPE=shared ./sundials_collect.sh "${SUNDIALS_BUILD}" ${sundials_modules}
#
# object dependencies
${OBJS} : sundials.h module_functions.h ${SOLVER_MODULES_BASE}/solver_modfcn.h
ida/ida_property.o cvode/cvode_property.o : ${MPT_BASE}/version.h
# target dependencies
${LIB_STATIC} ${LIB_SHARED} : sundials_collect.sh
