# mpt.solver.mk: solver library creation template
DIR_TOP  ?= ${MPT_PREFIX}
DIR_INC  ?= ${DIR_TOP}/include/mpt/solver
DIR_MATH ?= ${DIR_TOP}/math
#
# default header to export
HEADER ?= ${LIB}.h
#
# current directory
DIR_SOLVER_MODULES := $(dir $(lastword $(MAKEFILE_LIST)))
DIR_BASE ?= ${DIR_SOLVER_MODULES}../base/
#
# add directories to global include
INC += ${DIR_SOLVER_MODULES} ${DIR_BASE} ${DIR_BASE}mptcore
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
	modfcn_data_new.c modfcn_data_set.c \
	modfcn_ivp_state.c modfcn_ivp_vecset.c \
	modfcn_ivp_values.c
#
# math objects in different location
MATH_OBJS ?= $(MATH:%=${DIR_MATH}/%)
MATH_OBJS_STATIC ?= ${MATH_OBJS}
MATH_OBJS_SHARED ?= ${MATH_OBJS}
# object collections
STATIC_OBJS ?= ${OBJS} ${MATH_OBJS_STATIC}
SHLIB_OBJS ?= libinfo.o \
	${OBJS} ${MATH_OBJS_SHARED} \
	$(mod_require:%=${DIR_SOLVER_MODULES}mod_%.o)
#
# import library creation
include ${DIR_BASE}/mpt.lib.mk
CLEAN_FILES += libinfo.o
# compiler flags for FORTRAN math objects
FFLAGS ?= -fpic -O5 -Wall -fstack-protector
#
# additional object dependancies
$(mod_require:%=${DIR_SOLVER_MODULES}mod_%.o) \
${OBJS} : ${DIR_SOLVER_MODULES}../solver.h ${DIR_BASE}mptcore/meta.h ${MAKEFILE_LIST}
libinfo.o : ${DIR_BASE}libinfo.h ${DIR_BASE}version.h
#
# solver module configuration
.PHONY : module_config
install : module_config
module_config : ${CONFIG}; $(call install_files,${DIR_TOP}/etc/mpt.conf.d,${CONFIG})
CLEAR_FILES += $(CONFIG:%=${DIR_TOP}/etc/mpt.conf.d/%) libinfo.o
#
# module helper dependencies
*_modfcn.o : $(src_modfcn:%=${DIR_SOLVER_MODULES}%)
module_function.h : ${DIR_SOLVER_MODULES}solver_modfcn.h
#
# additional service targets
.PHONY : clean_math
clean_math :
	${RM} ${MATH_OBJS}

distclean : clean_math
#
