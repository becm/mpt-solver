# solver.example.mk: template for MPT solver examples
#
TESTS ?= ${PROGS}
#
# include general example rules
include $(dir $(lastword $(MAKEFILE_LIST)))../base/examples/mpt.example.mk
CLEAR_FILES += $(TESTS:%=%.out)
#
# default libraries for solver examples
libs = solver io
LDLIBS ?= $(libs:%=-lmpt%)
