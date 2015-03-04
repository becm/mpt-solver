# Makefile: nonlinear system client programs
#
TESTS = neter
OBJS  = main.o $(TESTS:%=%.o)

# general creation rules
all : ${PROGS} ${TESTS}

# use math library for neter target
neter : LDLIBS+= -lm

# general targets and definitions
include ../solver.example.mk

${TESTS} : main.o
neter : neter.o
