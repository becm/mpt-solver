# Makefile: MPT solver examples
#
DIRS = ode dae pde nls bacol
TESTS = solvers vecpar client
PROGS = loadsol ${TESTS}
solver_obj = main_ivp.o main_nls.o solver_run.o
OBJS = $(PROGS:%=%.o) $(TESTS:%=%.o) ${solver_obj}


CXXTARGETS = vecpar client solvers

include solver.example.mk

solvers = nlsolv sundials vode limex daesolv
aliases = ida limex bacol

vecpar vecpar_static : vecpar.o
vecpar : LDLIBS=-lmpt++
vecpar.o : CPPFLAGS+=-std=c++11

client client_static : client.o solver_run.o
client_static : LDLIBS=-lmptsolver -lmpt++ -lmpt
client : LDLIBS=-lmptsolver -lmpt++ -lmptio -lmptcore
test_client : ARGS=client.conf

solvers solvers_static : solvers.o
solvers : LDLIBS=-lmpt++ -lmptcore $(solvers:%=-lmpt_%)
solvers.o : CPPFLAGS+=-std=c++11 $(solvers:%=-Dwith_%)

loadsol :  LDLIBS=-lmptsolver -lmptloader -lmptcore
loadsol.o :
