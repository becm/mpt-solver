# Makefile: BACOL solver library
LIB ?= mpt_bacol
SOL ?= ivp
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
# sources and math objects
SRCS = $(wildcard bacol_*.c)
MATH = bacol.o bacolrr.o
#
# headers to export
HEADER = bacol.h
CONFIG = bacol.conf
#
# general library rules
include ../mpt.solver.mk
bacol_*.o : bacol.h
bacol_property.o libinfo.o : ${DIR_BASE}version.h
