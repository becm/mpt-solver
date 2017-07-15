# Makefile: ODE client programs
#
PROGS = crackoil
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${DIR_EXAMPLES}main_ivp.o ${DIR_EXAMPLES}solver_run.o

${DIR_EXAMPLES}*.o *.o : ${DIR_EXAMPLES}solver_run.h

crackoil : crackoil.o
test_crackoil : ARGS=crackoil.conf
