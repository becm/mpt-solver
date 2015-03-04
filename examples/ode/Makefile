# Makefile: ODE client programs
#
TESTS = crackoil
OBJS  = main.o $(TESTS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${TESTS} : main.o
crackoil : crackoil.o
