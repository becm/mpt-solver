# Makefile: DAE client programs
#
PROGS = robertson cakzo
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

# library requirements
cakzo: LDLIBS+=-lm

${PROGS}  : main.o
cakzo     : cakzo.o
robertson : robertson.o
