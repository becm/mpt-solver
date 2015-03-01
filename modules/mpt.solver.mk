# mpt.solver.mk: solver library creation template
DIR_INC  ?= ${MPT_PREFIX}/include/mpt/solver
DIR_MATH ?= ${MPT_PREFIX}/math
#
# default header to export
HEADER ?= ${LIB}.h
#
# current directory
DIR_SOLVER := $(dir $(lastword $(MAKEFILE_LIST)))
#
# add core to global include
INC += ${DIR_SOLVER}.. ${DIR_SOLVER} ${DIR_BASE} ${DIR_BASE}mptcore
#
# compiler flags for math objects
FFLAGS ?= -fpic -O5 -Wall
#
# math objects in different location
MATH_OBJS ?= $(MATH:%=${DIR_MATH}/%)
#
# vecpar and shared objects
src_gen = vecpar_alloc.c vecpar_property.c
src_ivp = ivppar_set.c vecpar_cktol.c
src_nls = nlspar_set.c
#
# additional objects for solver
ifeq (${SOL}, ivp)
  src_type = ${src_ivp} ${src_gen}
endif
ifeq (${SOL}, nls)
  src_type = ${src_nls} ${src_gen}
endif
ifeq (${SOL}, both)
  src_type = ${src_nls} ${src_ivp} ${src_gen}
endif
#
# special type objects
KEEP_OBJS  := $(src_type:%.c=${DIR_SOLVER}%.o) ${MATH_OBJS}
SHLIB_OBJS ?= libinfo.o

include ${DIR_SOLVER}../base/mpt.lib.mk

${OBJS} : ${DIR_SOLVER}../solver.h
libinfo.o : ${DIR_BASE}libinfo.h ${DIR_BASE}version.h release.h
#
# release information
include ${DIR_BASE}mpt.release.mk
#
# additional service targets
.PHONY : clean_math
clean_math :
	${RM} ${MATH_OBJS}

distclean : clean_math
