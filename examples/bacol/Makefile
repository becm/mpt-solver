# Makefile: MPT bacol examples
#
PROGS = sys4
OBJS  = main.o $(PROGS:%=%.o)

# general creation rules
all : ${PROGS}

# general targets and definitions
include ../solver.example.mk

${PROGS} : main.o

sys4 : LDFLAGS += -Wl,--export-dynamic
sys4 : sys4.o
