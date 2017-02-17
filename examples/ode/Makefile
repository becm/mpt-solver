# Makefile: ODE client programs
#
PROGS = crackoil
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ../main_ivp.o ../solver_run.o

crackoil : crackoil.o
test_crackoil : ARGS=crackoil.conf
