# Makefile: PDE client programs
#
TESTS = flame diff htrans twoeq makzo
OBJS  = main.o $(TESTS:%=%.o)

# general creation rules
all : ${PROGS} ${TESTS}

# additional libraries for targets
twoeq : LDLIBS+=-lmptcore
twoeq flame : LDLIBS+= -lm

# general targets and definitions
include ../solver.example.mk

# main skeleton for PDE
${TESTS} : main.o
flame  : flame.o
diff   : diff.o
htrans : htrans.o
twoeq  : twoeq.o
makzo  : makzo.o
