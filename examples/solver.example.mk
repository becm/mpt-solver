# solver.example.mk: template for MPT solver examples
#
TESTS ?= ${PROGS}
#
# specify and extend generic example rules
DIR_EXAMPLES := $(dir $(lastword $(MAKEFILE_LIST)))
DIR_SOLVER ?= ${DIR_EXAMPLES}../
DIR_BASE ?= ${DIR_SOLVER}base/
PREFIX ?= ${DIR_SOLVER}build
include ${DIR_BASE}/mpt.example.mk

INC ?= '${DIR_EXAMPLES}' '${DIR_SOLVER}' '${DIR_BASE}mptcore'

CLEAR_FILES += $(TESTS:%=%.out)
CLEAN_FILES += ${CLEAR_FILES}
#
# objects used in multiple binaries
${DIR_EXAMPLES}solver_run.o : ${DIR_EXAMPLES}solver_run.h
${DIR_EXAMPLES}solver_run.o : INC+='${DIR_BASE}mptio'
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
