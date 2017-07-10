# Makefile: LIMEX solver library
LIB ?= mpt_limex
SOL ?= ivp
#
# linker settings
LDLIBS = -llapack
LINK   = ${FC}
#
# sources and math objects
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
limex_*.o : limex.h limex_modfcn.h ${DIR_SOLVER_MODULES}solver_modfcn.h
limex_property.o :  ${DIR_BASE}version.h ${DIR_SOLVER_MODULES}solver_ivp_vecset.c
limex_ufcn.o : ${DIR_SOLVER_MODULES}solver_ufcn_dae.c
libinfo.o : ${DIR_BASE}version.h
