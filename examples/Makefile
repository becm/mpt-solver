# Makefile: MPT solver examples
#
DIRS = ode dae pde nls bacol
TESTS = solvers vecpar
PROGS = loadsol ${TESTS}
OBJS = $(PROGS:%=%.o) $(TESTS:%=%.o)

include solver.example.mk

solvers = nlsolv sundials vode limex daesolv
aliases = ida limex bacol

vecpar.o : CPPFLAGS+=-std=c++11
vecpar : CC=${CXX}

solvers : CC=${CXX}
solvers : LDLIBS=-lmptsolver -lmpt++ -lmptcore $(solvers:%=-lmpt_%)
solvers.o : CPPFLAGS+=-std=c++11 $(solvers:%=-Dwith_%)

loadsol :  LDLIBS=-lmptsolver -lmptio -lmptloader -lmptcore
