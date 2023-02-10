# Makefile: nonlinear system client programs
#
PROGS = neter
OBJS  = $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${DIR_EXAMPLES}solver_run.o

$(PROGS:%=%.o) : ../solver_run.h

neter : neter.o
neter neter_static : LDLIBS+=-lm
test_neter : ARGS=neter.conf

test_% : MPT_FLAGS=e
