# Makefile: nonlinear system client programs
#
PROGS = neter
OBJS  = $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${EXAMPLES_BASE}/solver_run.o

$(PROGS:%=%.o) : ${EXAMPLES_BASE}/solver_run.h

neter : neter.o
neter neter_static : LDLIBS+=-lm
test_neter : ARGS=neter.conf

test_% : MPT_FLAGS=e
