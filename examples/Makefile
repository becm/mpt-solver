# Makefile: MPT solver examples
#
DIRS  = ode dae pde nls bacol
PROGS = config
TESTS =

include solver.example.mk

config : config.o
config : LDLIBS+=-lmptcore
