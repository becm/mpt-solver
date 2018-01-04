# Makefile: MPT solver library
LIB = mptsolver
#
# additional header directories
base_inc = . mptcore mptplot
INC = .. $(base_inc:%=${DIR_BASE}%)
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
SRCS := $(wildcard conf_*.c output_*.c solver_*.c) \
	$(wildcard ivp/*.c nls/*.c) \
	timeradd.c
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${DIR_TOP}/include/mpt
DIR_BASE = ../base/
include ${DIR_BASE}mpt.lib.mk
#
# additional dependencies
libinfo.o : ${DIR_BASE}libinfo.h ${DIR_BASE}version.h
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
