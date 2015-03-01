# solver.example.mk: template for MPT solver examples
#
# include general example rules
include $(dir $(lastword $(MAKEFILE_LIST)))../base/examples/mpt.example.mk
#
# default libraries for solver examples
libs = solver client plot core
LDLIBS ?= $(libs:%=-lmpt%)
