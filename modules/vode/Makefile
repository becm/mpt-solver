# Makefile: dVODE solver library
LIB ?= mpt_vode
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
# shared components
mod_require = ${mod_ivp} ufcn_ode
#
# module components
MATH = vode.o dgbsl.o dgesl.o dgbfa.o dgefa.o
SRCS = $(wildcard vode_*.c)
#
# headers to export
HEADER = vode.h
CONFIG = vode.conf
#
# solver rules
include ../mpt.solver.mk
vode_*.o : vode.h module_functions.h ${DIR_SOLVER_MODULES}solver_modfcn.h
vode_property.o : ${DIR_BASE}version.h
