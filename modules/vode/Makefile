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
SRCS = $(wildcard vode_*.c)
#
# headers to export
HEADER = vode.h
CONFIG = vode.conf
#
# solver rules
include ../mpt.solver.mk
vode_*.o : vode_modfcn.h
vode_*.o vode_modfcn.h : ${DIR_SOLVER_MODULES}solver_modfcn.h
vode_property.o : ${DIR_BASE}version.h
vode_ufcn.o : ${DIR_SOLVER_MODULES}solver_odefcn.c
