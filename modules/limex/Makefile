# Makefile: LIMEX solver library
LIB ?= mpt_limex
#
# linker settings
LDLIBS = -llapack
LINK   = ${FC}
#
# shared components
mod_require = ${mod_ivp} ufcn_dae
#
# module components
SRCS := $(wildcard limex_*.c)
MATH =  zib/LIMEX4_2A1.o zib/LIMEX4_2_Dense.o
#
# headers to export
HEADER = limex.h
CONFIG = limex.conf
#
# general library rules
include ../mpt.solver.mk
#
# local dependencies
${OBJS} : limex.h module_functions.h ${DIR_SOLVER_MODULES}solver_modfcn.h
limex_property.o : ${DIR_BASE}version.h
