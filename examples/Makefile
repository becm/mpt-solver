# Makefile: MPT solver examples
#
DIRS  = ode dae pde nls bacol
TESTS = check proxy
PROGS = config ${TESTS}
STATIC = 
OBJS  = $(PROGS:%=%.o) $(TESTS:%=%.o)

include solver.example.mk
CLEAR_FILES += ${TESTS}

config : config.o
config : LDLIBS=-lmptsolver -lmptloader -lmptcore

solvers = nlsolv sundials vode limex daesolv

check.o : CPPFLAGS+=-std=c++11
check : CC=${CXX}
check : LDLIBS=-lmptsolver -lmptcore $(solvers:%=-lmpt_%)

proxy :  LDLIBS=-lmptsolver
proxy_static : LDLIBS=-lmptsolver -lmpt -ldl
