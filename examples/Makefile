# Makefile: MPT solver examples
#
DIRS = ode dae pde nls bacol
TESTS = loadsol solvers vecpar
PROGS = config ${TESTS}
STATIC = loadsol
OBJS = $(PROGS:%=%.o) $(TESTS:%=%.o)

include solver.example.mk

solvers = nlsolv sundials vode limex daesolv
aliases = ida limex bacol

config : config.o
config : LDLIBS=-lmptsolver -lmptloader -lmptcore
config_static : LDLIBS=-lmptsolver -lmpt -ldl

vecpar.o : CPPFLAGS+=-std=c++11
vecpar : CC=${CXX}

solvers : CC=${CXX}
solvers : LDLIBS=-lmptsolver -lmpt++ -lmptcore $(solvers:%=-lmpt_%)
solvers.o : CPPFLAGS+=-std=c++11 $(solvers:%=-Dwith_%)

loadsol :  LDLIBS=-lmptloader -lmptcore
loadsol_static : LDLIBS=-lmpt -ldl
