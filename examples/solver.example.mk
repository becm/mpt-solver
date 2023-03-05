# solver.example.mk: template for MPT solver examples
#
TESTS ?= ${PROGS}
#
# specify and extend generic example rules
EXAMPLES_BASE := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
SOLVER_BASE ?= $(abspath ${EXAMPLES_BASE}/..)
SOLVER_MODULES_BASE ?= ${SOLVER_BASE}/modules
PREFIX ?= ${SOLVER_BASE}/build
include ${SOLVER_BASE}/base/mpt.example.mk

INC ?= '${EXAMPLES_BASE}' '${SOLVER_BASE}' '${MPT_BASE}/mptcore'

CLEAR_FILES += $(TESTS:%=%.out)
CLEAN_FILES += ${CLEAR_FILES}
#
# objects used in multiple binaries
${EXAMPLES_BASE}/solver_run.o : ${EXAMPLES_BASE}/solver_run.h
${EXAMPLES_BASE}/solver_run.o : INC+='${MPT_BASE}/mptio'
#
# default libraries for static/shared builds
${STATIC} : libs=mptsolver mpt dl
${PROGS} : libs=mptsolver mptio mptcore
LDLIBS ?= $(libs:%=-l%)
#
# use C++ linker for relevant targets
${CXXTARGETS}:
	${CXX} ${LDFLAGS} ${^} ${LDLIBS} -o ${@}
$(CXXTARGETS:%=%_static):
	${CXX} -static ${LDFLAGS} ${^} ${LDLIBS} -o ${@}
