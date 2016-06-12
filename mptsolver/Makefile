# Makefile: MPT solver library
LIB = mptsolver
#
# additional header directories
base_inc = . mptcore mptplot
INC = .. $(base_inc:%=../base/%)
#
# header to export
HEADER = ../solver.h
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS = libinfo.o ${OBJS}
LDLIBS = -lmptplot -lmptloader -lmptcore
#
# source summary
SRCS := $(wildcard client_*.c conf_*.c data_*.c solver_*.c output_*.c init_*.c) \
	timeradd.c steps_ode.c residuals_cdiff.c
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${DIR_TOP}/include/mpt
include ../base/mpt.lib.mk
#
# additional dependencies
libinfo.o : ../base/libinfo.h ../base/version.h
CLEAR_FILES += libinfo.o
