# Makefile: MPT solver examples
#
DIRS  = ode dae pde nls bacol
TESTS = check proxy
PROGS = config ${TESTS}
STATIC = proxy
OBJS  = $(PROGS:%=%.o) $(TESTS:%=%.o)

include solver.example.mk

config : config.o
config : LDLIBS=-lmptsolver -lmptloader -lmptcore
config_static : LDLIBS=-lmptsolver -lmpt -ldl

solvers = nlsolv sundials vode limex daesolv
aliases = ida limex bacol

check.o : CPPFLAGS+=-std=c++11
check : CC=${CXX}
check : LDLIBS=-lmptsolver -lmptcore $(solvers:%=-lmpt_%)

proxy :  LDLIBS=-lmptsolver -lmptcore
proxy_static : LDLIBS=-lmptsolver -lmpt -ldl
