# solver.example.mk: template for MPT solver examples
#
TESTS ?= ${PROGS}
#
# include general example rules
DIR_TOP = ${MPT_PREFIX}
include $(dir $(lastword $(MAKEFILE_LIST)))../base/mpt.example.mk
CLEAR_FILES += $(TESTS:%=%.out)
#
# default libraries for static/shared builds
${STATIC} : libs=mptsolver mpt dl
${PROGS} : libs=mptsolver mptio
LDLIBS ?= $(libs:%=-l%)
