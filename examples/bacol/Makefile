# Makefile: MPT bacol examples
#
PROGS = sys4
OBJS  = sys4_fcn.o $(PROGS:%=%.o)

# no static option
STATIC =

# general targets and definitions
include ../solver.example.mk

${PROGS} : ${DIR_EXAMPLES}main_ivp.o ${DIR_EXAMPLES}solver_run.o

${DIR_EXAMPLES}*.o *.o : ${DIR_EXAMPLES}solver_run.h

sys4 : sys4.o sys4_fcn.o
sys4 : LDLIBS+=-lmptcore
sys4 : LDFLAGS+=-Wl,--export-dynamic
test_sys4 : ARGS=sys4.conf
