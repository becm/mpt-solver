# Makefile: DAE solver library
LIB ?= mpt_daesolv
SOL ?= ivp
#
# add local directory to search path
INC = .
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
# sources and objects
dassl_sources := $(wildcard dassl/dassl_*.c)
radau_sources := $(wildcard radau/radau_*.c)
mebdfi_sources := $(wildcard mebdfi/mebdfi_*.c)
SRCS = ${dassl_sources} ${radau_sources} ${mebdfi_sources} daesolv_ufcn.c daesolv_ivpvec_set.c
MATH = \
	mebdfi.o \
	radau.o radaua.o \
	ddassl.o \
	dgbfa.o dgbsl.o \
	dgefa.o dgesl.o
#
# headers to export
HEADER = mebdfi.h radau.h dassl.h
CONFIG = daesolv.conf
#
# general library rules
include ../mpt.solver.mk
#
# special compiler flags for MEBDFI
${DIR_MATH}/mebdfi.o : FFLAGS=-O0 -fpic -Wall
#
# object dependencies
libinfo.o : ${HEADER}
dassl/*.o : dassl.h
radau/*.o : radau.h
mebdfi/*.o : mebdfi.h
${OBJS} : daesolv_modfcn.h ${DIR_SOLVER_MODULES}solver_modfcn.h ${DIR_BASE}version.h
daesolv_ufcn.o : ${DIR_SOLVER_MODULES}solver_daefcn.c
daesolv_ivpvec.o : ${DIR_SOLVER_MODULES}solver_ivpvec.c
