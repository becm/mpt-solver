# Makefile: nonlinear system client programs
#
PROGS = neter
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${DIR_EXAMPLES}solver_run.o

${DIR_EXAMPLES}*.o *.o : ${DIR_EXAMPLES}solver_run.h

neter : neter.o
neter neter_static : LDLIBS+=-lm
test_neter : ARGS=neter.conf
