# Makefile: nonlinear system client programs
#
PROGS = neter
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ../main_nls.o ../solver_run.o

../solver_run.o : ../solver_run.h
*.o : ../solver_ivp.h

neter : neter.o
neter neter_static : LDLIBS+=-lm
test_neter : ARGS=neter.conf
