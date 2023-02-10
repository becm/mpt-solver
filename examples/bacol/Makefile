# Makefile: MPT bacol examples
#
PROGS = sys4 bacol
OBJS  = sys4_fcn.o $(PROGS:%=%.o)

# use C++ to link executable
CXXTARGETS = bacol

# general targets and definitions
include ../solver.example.mk

sys4 sys4_static : ${DIR_EXAMPLES}solver_run.o

sys4.o : ${DIR_EXAMPLES}solver_run.h

sys4 : sys4.o sys4_fcn.o
sys4 : LDLIBS+=-lmptcore
sys4 : LDFLAGS+=-Wl,--export-dynamic
test_sys4 : ARGS=sys4.conf

bacol : bacol.o sys4_fcn.o
bacol : LDLIBS=-lmpt_bacol -lmptsolver -lmptcore
bacol.o : INC+='${DIR_SOLVER}modules/bacol'

test_% : MPT_FLAGS=e
