# Makefile: nonlinear system client programs
#
PROGS = neter
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

# use math library for neter target
neter : LDLIBS+= -lm

${PROGS} : main.o
neter : neter.o
