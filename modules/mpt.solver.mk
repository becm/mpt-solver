# mpt.solver.mk: solver module creation template
SOLVER_MODULES_BASE := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
SOLVER_BASE ?= $(abspath ${SOLVER_MODULES_BASE}/..)
#
# default source directories
MPT_BASE ?= ${SOLVER_BASE}/base
MATH_BASE ?= ${SOLVER_BASE}/math
#
# relative default target directories
PREFIX ?= ${SOLVER_BASE}/build
PREFIX_INC ?= ${PREFIX}/include/mpt/solver
#
# default header to export
HEADER ?= ${LIB}.h
#
# add directories to global include
INC += '${SOLVER_MODULES_BASE}' '${MPT_BASE}' '${MPT_BASE}/mptcore'
#
# vecpar and other shared operations
mod_tol = tol_check tol_set tol_get
mod_ivp = ivpset nextval ${mod_tol} valloc value consume_value report_properties
mod_nls = nlsset valloc value consume_value report_properties
mod_sol = nlsset ${mod_ivp}
#
# module function templates
src_modfcn = \
	modfcn_solver_conv.c \
	modfcn_data_new.c \
	modfcn_data_set.c \
	modfcn_ivp_state.c \
	modfcn_ivp_vecset.c \
	modfcn_ivp_values.c
#
# math objects in different location
MATH_OBJS ?= $(MATH:%=${MATH_BASE}/%)
MATH_OBJS_STATIC ?= ${MATH_OBJS}
MATH_OBJS_SHARED ?= ${MATH_OBJS}
# object collections
STATIC_OBJS ?= ${OBJS} ${MATH_OBJS_STATIC}
SHLIB_OBJS ?= libinfo.o \
	${OBJS} ${MATH_OBJS_SHARED} \
	$(mod_require:%=${SOLVER_MODULES_BASE}/mod_%.o)
#
# import library creation
include ${MPT_BASE}/mpt.lib.mk
CLEAN_FILES += libinfo.o
# compiler flags for FORTRAN math objects
FFLAGS ?= -fpic -O5 -Wall -fstack-protector
#
# additional object dependancies
$(mod_require:%=${SOLVER_MODULES_BASE}/mod_%.o) \
${OBJS} : ${SOLVER_MODULES_BASE}/../solver.h ${MPT_BASE}/mptcore/meta.h ${MAKEFILE_LIST}
libinfo.o : ${MPT_BASE}/libinfo.h ${MPT_BASE}/version.h
#
# solver module configuration
.PHONY : module_config
install : module_config
module_config : ${CONFIG}; $(call install_files,${PREFIX}/etc/mpt.conf.d,${CONFIG})
CLEAR_FILES += $(CONFIG:%=${PREFIX}/etc/mpt.conf.d/%) libinfo.o
#
# module helper dependencies
*_modfcn.o : $(src_modfcn:%=${SOLVER_MODULES_BASE}/%)
module_function.h : ${SOLVER_MODULES_BASE}/solver_modfcn.h
#
# additional service targets
.PHONY : clean_math
clean_math :
	${RM} ${MATH_OBJS}

distclean : clean_math
#
