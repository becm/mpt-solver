# Makefile: MPT solver examples
#
DIRS = ode dae pde nls bacol
TESTS = solvers vecpar client
PROGS = loadsol ${TESTS}
solver_obj = main_ivp.o main_nls.o solver_run.o
OBJS = $(PROGS:%=%.o) $(TESTS:%=%.o) ${solver_obj}

include solver.example.mk

solvers = nlsolv sundials vode limex daesolv
aliases = ida limex bacol

vecpar : CC=${CXX}
vecpar : LDLIBS=-lmpt++
vecpar.o : CPPFLAGS+=-std=c++11

client : CC=${CXX}
client : client.o solver_run.o
client : LDLIBS=-lmptsolver -lmpt++ -lmptio -lmptcore
test_client : ARGS=client.conf

solvers : CC=${CXX}
solvers : LDLIBS=-lmpt++ -lmptcore $(solvers:%=-lmpt_%)
solvers.o : CPPFLAGS+=-std=c++11 $(solvers:%=-Dwith_%)

loadsol :  LDLIBS=-lmptsolver -lmptloader -lmptcore
loadsol.o :

