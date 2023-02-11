# Makefile: collection of standalone nonlinear Solvers
LIB ?= mpt_nlsolv
#
# add local directory to search path
INC = .
#
# shared library version
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
# linker settings
LINK = ${FC}
#
# shared components
mod_require = ${mod_nls} ufcn_nls
#
# import solver settings
include portn2/portdn2.mk
include minpack/minpack.mk
-include direct/direct.mk

MATH = \
	$(portdn2_math:%=port/%) \
	$(minpack_math:%=minpack/%) \
	${direct_math}

OBJS = \
	$(portn2_wrapper:%=portn2/%) \
	$(minpack_wrapper:%=minpack/%) \
	$(direct_wrapper:%=direct/%) \
	nlsolv_modfcn.o

HEADER = minpack/minpack.h portn2/portn2.h
CONFIG = nlsolv.conf
#
# general solver build settings
include ../mpt.solver.mk
#
# local object dependencies
${OBJS} : module_functions.h ${SOLVER_MODULES_BASE}/solver_modfcn.h
$(portn2_wrapper:%=portn2/%)   : portn2/portn2.h
$(minpack_wrapper:%=minpack/%) : minpack/minpack.h
minpack/minpack_property.o portn2/portn2_property.o  : ${MPT_BASE}/version.h
