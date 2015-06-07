# Makefile: DAE client programs
#
PROGS = robertson cakzo
OBJS  = main.o $(PROGS:%=%.o)

# general creation rules
all : ${PROGS}

# general targets and definitions
include ../solver.example.mk

${PROGS} : main.o

cakzo : LDLIBS+=-lm
cakzo : cakzo.o

robertson : robertson.o
