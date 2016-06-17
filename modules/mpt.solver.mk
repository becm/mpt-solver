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
src_ivp = ivppar_set.c vecpar_cktol.c vecpar_settol.c
src_nls = nlspar_set.c
#
# additional objects for solver
ifeq (${SOL}, ivp)
  src_solver = ${src_ivp} ${src_gen}
endif
ifeq (${SOL}, nls)
  src_solver = ${src_nls} ${src_gen}
endif
ifeq (${SOL}, both)
  src_solver = ${src_nls} ${src_ivp} ${src_gen}
endif
#
# math objects in different location
MATH_OBJS ?= $(MATH:%=${DIR_MATH}/%)
MATH_OBJS_STATIC ?= ${MATH_OBJS}
MATH_OBJS_SHARED ?= ${MATH_OBJS}
# object collections
default_objects = ${OBJS} $(src_solver:%.c=${DIR_SOLVER}%.o)
STATIC_OBJS ?= ${default_objects} ${MATH_OBJS_STATIC}
SHLIB_OBJS ?= libinfo.o ${default_objects} ${MATH_OBJS_SHARED}
#
# import library creation
include ${DIR_SOLVER}../base/mpt.lib.mk
# compiler flags for FORTRAN math objects
FFLAGS ?= -fpic -O5 -Wall -fstack-protector
#
# additional object dependancies
${OBJS} : ${DIR_SOLVER}../solver.h
libinfo.o : ${DIR_BASE}libinfo.h ${DIR_BASE}version.h
#
# solver module configuration
.PHONY : module_config
shared install : module_config
module_config : ${CONFIG}; $(call install_files,${DIR_TOP}/etc/mpt.conf.d,${CONFIG})
CLEAR_FILES += $(CONFIG:%=${DIR_TOP}/etc/mpt.conf.d/%) libinfo.o
#
# additional service targets
.PHONY : clean_math
clean_math :
	${RM} ${MATH_OBJS}

distclean : clean_math
#
