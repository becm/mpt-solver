# Makefile: MPT solver examples
#
DIRS  = ode dae pde nls bacol
PROGS = config
TESTS =
OBJS  = $(PROGS:%=%.o)

include solver.example.mk

config : config.o
config : LDLIBS=-lmptsolver -lmptloader -lmptcore
