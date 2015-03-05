# Makefile: MPT bacol examples
#
TESTS = sys4
OBJS  = main.o $(TESTS:%=%.o)

# general creation rules
all : ${PROGS} ${TESTS}

# equotations, wrapper settings
${TESTS} : sys4.o

# general targets and definitions
include ../solver.example.mk

# PDE client main
sys4 : LDFLAGS += -Wl,--export-dynamic
sys4 : main.o
