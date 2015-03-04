# Makefile: DAE client programs
#
TESTS = robertson cakzo
OBJS  = main.o $(TESTS:%=%.o)

# general creation rules
all : ${PROGS} ${TESTS}

# library requirements
cakzo: LDLIBS+=-lm

# general targets and definitions
include ../solver.example.mk

${TESTS}  : main.o
cakzo     : cakzo.o
robertson : robertson.o
