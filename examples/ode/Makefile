# Makefile: ODE client programs
#
PROGS = crackoil
OBJS  = $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${DIR_EXAMPLES}solver_run.o

$(PROGS:%=%.o) : ../solver_run.h

crackoil : crackoil.o
test_crackoil : ARGS=crackoil.conf

test_% : MPT_FLAGS=e
