# solver.example.mk: template for MPT solver examples
#
TESTS ?= ${PROGS}
#
# include general example rules
DIR_TOP = ${MPT_PREFIX}
DIR_BASE ?= $(dir $(lastword $(MAKEFILE_LIST)))../base
include ${DIR_BASE}/mpt.example.mk
CLEAR_FILES += $(TESTS:%=%.out)
CLEAN_FILES += ${CLEAR_FILES}
#
# default libraries for static/shared builds
${STATIC} : libs=mptsolver mpt dl
${PROGS} : libs=mptsolver mptio
LDLIBS ?= $(libs:%=-l%)
