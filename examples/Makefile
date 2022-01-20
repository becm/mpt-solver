# Makefile: MPT solver examples
#
DIRS = ode dae pde nls bacol
TESTS = client output solvers vecpar
PROGS = loadsol ${TESTS}
solver_obj = main_ivp.o main_nls.o solver_run.o
OBJS = $(PROGS:%=%.o) $(TESTS:%=%.o) ${solver_obj}


CXXTARGETS = client output solvers vecpar

include solver.example.mk

ivp_solvers = sundials vode limex daesolv
nl_solvers = minpack portn2
solvers = ${ivp_solvers} nlsolv

vecpar vecpar_static : vecpar.o
vecpar : LDLIBS=-lmpt++
vecpar.o : CPPFLAGS+=-std=c++11

client client_static : client.o solver_run.o
client_static : LDLIBS=-lmptsolver -lmpt++ -lmpt -ldl
client : LDLIBS=-lmptsolver -lmpt++ -lmptio -lmptcore

output output_static : output.o
output : LDLIBS=-lmpt++
output_static : LDLIBS=-lmpt++ -lmpt
output.o : CPPFLAGS+=-std=c++11
output.o : INC+='${DIR_BASE}mptplot'

solvers solvers_static : solvers.o
solvers : LDLIBS=-lmpt++ -lmptcore $(solvers:%=-lmpt_%)
solvers_static : LDLIBS=-lmpt++ -lmpt -lmpt_solvers
solvers.o : CPPFLAGS+=-std=c++11 $(solvers:%=-Dwith_%)
solvers.o : INC+=$(ivp_solvers:%='${DIR_SOLVER}modules/%')
solvers.o : INC+=$(nl_solvers:%='${DIR_SOLVER}modules/nlsolv/%')

loadsol :  LDLIBS=-lmptsolver -lmptloader -lmptcore
loadsol.o :
