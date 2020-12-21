# Makefile: BACOL solver library
LIB ?= mpt_bacol
#
# additional build flags
DEF = MPT_BACOL_DASSL MPT_BACOL_RADAU
#
# shared library version
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
# linker settings
LDLIBS = -lblas
LINK   = ${FC}
#
# module helpers
mod_require = ${mod_ivp}
#
# sources and math objects
SRCS = $(wildcard bacol_*.c)
MATH = \
	bacolnoblas.o \
	bacolrr.o \
	dgbsl.o dgbfa.o \
	dgesl.o dgefa.o
#
# headers to export
HEADER = bacol.h
CONFIG = bacol.conf
#
# general library rules
include ../mpt.solver.mk
bacol_*.o : bacol.h
bacol_property.o : ${DIR_BASE}version.h

