# Makefile: ODE client programs
#
PROGS = crackoil
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : main.o

crackoil : crackoil.o
test_crackoil : ARGS=crackoil.conf
