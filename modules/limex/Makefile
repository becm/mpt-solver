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
#
# general library rules
include ../mpt.solver.mk
#
# local dependencies
limex_property.o : ${DIR_BASE}version.h release.h
