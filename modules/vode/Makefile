# Makefile: dVODE solver library
LIB ?= mpt_vode
SOL ?= ivp
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
# sources and math objects
MATH = vode.o dgbsl.o dgesl.o dgbfa.o dgefa.o
SRCS = $(wildcard vode_*.c) ../mod_ufcn_ode.c
#
# headers to export
HEADER = vode.h
CONFIG = vode.conf
#
# solver rules
include ../mpt.solver.mk
vode_*.o : vode.h module_functions.h ${DIR_SOLVER_MODULES}solver_modfcn.h
vode_property.o libinfo.o : ${DIR_BASE}version.h
