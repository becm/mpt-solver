# solver.example.mk: template for MPT solver examples
#
TESTS ?= ${PROGS}
#
# include general example rules
DIR_TOP = ${MPT_PREFIX}
DIR_EXAMPLES := $(dir $(lastword $(MAKEFILE_LIST)))
DIR_BASE ?= ${DIR_EXAMPLES}../base
include ${DIR_BASE}/mpt.example.mk

CFLAGS += -I..
CLEAR_FILES += $(TESTS:%=%.out)
CLEAN_FILES += ${CLEAR_FILES}
#
# default libraries for static/shared builds
${STATIC} : libs=mptsolver mpt dl
${PROGS} : libs=mptsolver mptio mptcore
LDLIBS ?= $(libs:%=-l%)
