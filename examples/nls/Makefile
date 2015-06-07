# Makefile: nonlinear system client programs
#
PROGS = neter
OBJS  = main.o $(PROGS:%=%.o)

# general creation rules
all : ${PROGS}

# general targets and definitions
include ../solver.example.mk

${PROGS} : main.o

neter : LDLIBS+= -lm
neter : neter.o
