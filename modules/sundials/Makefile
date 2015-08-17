# Makefile: SUNDIALS interface library
LIB ?= mpt_sundials
SOL ?= both
#
# additional build flags
# DEFINES += SUNDIALS_WITH_LAPACK=1
DIR_SUNDIALS ?= ${MPT_PREFIX}/math/sundials-2.6.2/build
sundials_elements = cvode ida kinsol nvec_ser
INC = . ${MPT_PREFIX}/include ${DIR_SUNDIALS}/include/
#
# shared library version
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
# linker settings
# LDLIBS += -llapack -lblas
LDLIBS += -lm
LINK   = ${CC}
#
# objects
sundials_sources = ewtfcn.c nvector.c jacobian.c report.c
cvode_sources    = $(wildcard cvode/cvode_*.c)
ida_sources      = $(wildcard ida/ida_*.c)

SRCS := ${sundials_sources} ${cvode_sources} ${ida_sources}

MATH_OBJS_STATIC != env DIR_SUNDIALS="${DIR_SUNDIALS}" LIBRARY_TYPE=static ./sundials_collect.sh ${sundials_elements}
MATH_OBJS_SHARED != env DIR_SUNDIALS="${DIR_SUNDIALS}" LIBRARY_TYPE=shared ./sundials_collect.sh ${sundials_elements}
MATH_OBJS = ${MATH_OBJS_STATIC} ${MATH_OBJS_SHARED}
#
# headers to export
HEADER = sundials.h
#
# general library rules
include ../mpt.solver.mk
#
# object dependencies
${OBJS} : sundials.h
ida/ida_property.o cvode/cvode_property.o : ${DIR_BASE}version.h release.h
