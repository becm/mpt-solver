# Makefile: SUNDIALS interface library
LIB ?= mpt_sundials
#
# base location of SUNDIALS artifacts
sundials_version = 5.8.0
DIR_SUNDIALS ?= ${MPT_PREFIX}/math/sundials-${sundials_version}
#
# precompiled object location
DIR_SUNDIALS_OBJ ?= ${DIR_SUNDIALS}/build
# include directories (generated, public)
DIR_SUNDIALS_INC ?= ${DIR_SUNDIALS_OBJ}/include ${DIR_SUNDIALS}/include
INC = . ${DIR_SUNDIALS_INC}
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

MATH_OBJS_STATIC != env DIR_SUNDIALS="${DIR_SUNDIALS_OBJ}" LIBRARY_TYPE=static ./sundials_collect.sh ${sundials_modules}
MATH_OBJS_SHARED != env DIR_SUNDIALS="${DIR_SUNDIALS_OBJ}" LIBRARY_TYPE=shared ./sundials_collect.sh ${sundials_modules}
#
# headers to export
HEADER = sundials.h
CONFIG = sundials.conf
#
# general library rules
include ../mpt.solver.mk
#
# object dependencies
${OBJS} : sundials.h module_functions.h ${DIR_SOLVER_MODULES}solver_modfcn.h
ida/ida_property.o cvode/cvode_property.o : ${DIR_BASE}version.h
# target dependencies
${LIB_STATIC} ${LIB_SHARED} : sundials_collect.sh
