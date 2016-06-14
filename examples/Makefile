# Makefile: MPT solver examples
#
DIRS  = ode dae pde nls bacol
PROGS = config
TESTS = check data
OBJS  = $(PROGS:%=%.o) $(TESTS:%=%.o)

include solver.example.mk
CLEAR_FILES += ${TESTS}

config : config.o
config : LDLIBS=-lmptsolver -lmptloader -lmptcore

solvers = nlsolv sundials vode limex daesolv

check check.o : CPPFLAGS+=-std=c++14
check : CC=${CXX}
check : LDLIBS=-lmptsolver -lmptcore $(solvers:%=-lmpt_%)

data data.o data_static : CPPFLAGS+=-std=c++14
data : CC=${CXX}
data : LDLIBS=-lmpt++ -lmptloader
