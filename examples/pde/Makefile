# Makefile: PDE client programs
#
PROGS = flame diff htrans twoeq makzo
OBJS  = main.o $(PROGS:%=%.o)

# general creation rules
all : ${PROGS}

# general targets and definitions
include ../solver.example.mk

${PROGS} : main.o

flame : LDLIBS+=-lm
flame : flame.o

diff   : diff.o

htrans : htrans.o

twoeq : LDLIBS+=-lmptcore -lm
twoeq : twoeq.o

makzo : makzo.o
