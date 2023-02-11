# Makefile: ODE client programs
#
PROGS = crackoil
OBJS  = $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${EXAMPLES_BASE}/solver_run.o

$(PROGS:%=%.o) : ${EXAMPLES_BASE}/solver_run.h

crackoil : crackoil.o
test_crackoil : ARGS=crackoil.conf

test_% : MPT_FLAGS=e
