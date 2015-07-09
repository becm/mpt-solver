# mpt.solver.mk: solver library creation template
DIR_TOP  ?= ${MPT_PREFIX}
DIR_INC  ?= ${DIR_TOP}/include/mpt/solver
DIR_MATH ?= ${DIR_TOP}/math
#
# default header to export
HEADER ?= ${LIB}.h
#
# current directory
DIR_SOLVER := $(dir $(lastword $(MAKEFILE_LIST)))
#
# add core to global include
INC += ${DIR_SOLVER}.. ${DIR_BASE} ${DIR_BASE}mptcore
#
# vecpar and shared objects
src_gen = vecpar_alloc.c vecpar_value.c
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
# math objects in different location
MATH_OBJS ?= $(MATH:%=${DIR_MATH}/%)
# object collections
default_objects = ${OBJS} $(src_type:%.c=${DIR_SOLVER}%.o) ${MATH_OBJS}
STATIC_OBJS ?= ${default_objects}
SHLIB_OBJS ?= libinfo.o ${default_objects}
#
# import library creation
include ${DIR_SOLVER}../base/mpt.lib.mk
# compiler flags for FORTRAN math objects
FFLAGS ?= -fpic -O5 -Wall -fstack-protector
#
# additional object dependancies
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
#
