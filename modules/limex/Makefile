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
limex_*.o libinfo.o : limex.h limex_modfcn.h ${DIR_SOLVER_MODULES}solver_modfcn.h
vode_property.o libinfo.o : ${DIR_BASE}version.h
limex_ufcn.o : ${DIR_SOLVER_MODULES}solver_daefcn.c
limex_ivpvec_set.o : ${DIR_SOLVER_MODULES}solver_ivpvec_set.c
