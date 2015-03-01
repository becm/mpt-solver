# Makefile: PDE client programs
#
PROGS = flame diff htrans twoeq makzo
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

# use math library for twoeq target
twoeq flame : LDLIBS+= -lm

# main skeleton for PDE
${PROGS} : main.o
flame  : flame.o
diff   : diff.o
htrans : htrans.o
twoeq  : twoeq.o
makzo  : makzo.o
