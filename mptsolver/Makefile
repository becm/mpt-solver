# Makefile: MPT solver library
LIB = mptsolver
#
# additional header directories
base_inc = . mptcore mptplot mptloader
INC = .. $(base_inc:%='${MPT_BASE}/%')
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
PREFIX ?= $(dir $(MAKEFILE))../build
PREFIX_INC ?= ${PREFIX}/include/mpt
include $(dir $(MAKEFILE))../base/mpt.lib.mk
#
# additional dependencies
libinfo.o : ${MPT_BASE}/libinfo.h ${MPT_BASE}/version.h
CLEAR_FILES += libinfo.o
CLEAN_FILES += libinfo.o
