# Makefile: SUNDIALS interface library
LIB ?= mpt_sundials
#
# precompiled object location
sundials_version = 4.0.0
DIR_SUNDIALS_OBJ ?= ${MPT_PREFIX}/math/sundials-${sundials_version}/build
# include directories
DIR_SUNDIALS_INC ?= ${MPT_PREFIX}/include
INC = . ${DIR_SUNDIALS_INC}
#
# SUNDIALS modules to include in shared library
sundials_lapack = sunlinsol_lapackband sunlinsol_lapackdense
sundials_direct = sunlinsol_band sunlinsol_dense ${sundials_lapack}
sundials_elements = cvode ida kinsol ${sundials_iter} ${sundials_direct} nvec_ser
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

MATH_OBJS_STATIC != env DIR_SUNDIALS="${DIR_SUNDIALS_OBJ}" LIBRARY_TYPE=static ./sundials_collect.sh ${sundials_elements}
MATH_OBJS_SHARED != env DIR_SUNDIALS="${DIR_SUNDIALS_OBJ}" LIBRARY_TYPE=shared ./sundials_collect.sh ${sundials_elements}
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
ida/ida_property.o cvode/cvode_property.o libinfo.o : ${DIR_BASE}version.h
