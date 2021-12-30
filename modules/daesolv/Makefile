# Makefile: DAE solver library
LIB ?= mpt_daesolv
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
# module helpers
mod_require = ${mod_ivp} ufcn_dae
#
# module components
dassl_sources := $(wildcard dassl/dassl_*.c)
radau_sources := $(wildcard radau/radau_*.c)
mebdfi_sources := $(wildcard mebdfi/mebdfi_*.c)
SRCS = \
	${dassl_sources} \
	${radau_sources} \
	${mebdfi_sources} \
	daesolv_modfcn.c
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
# object dependencies
${OBJS} : module_functions.h ${DIR_SOLVER_MODULES}solver_modfcn.h
*/*_property.o : ${DIR_BASE}version.h
dassl/*.o : dassl.h
radau/*.o : radau.h
mebdfi/*.o : mebdfi.h
