# Makefile: MPT bacol examples
#
PROGS = sys4
OBJS  = main.o $(PROGS:%=%.o)

# no static option
STATIC =

# general targets and definitions
include ../solver.example.mk

${PROGS} : main.o

sys4 : sys4.o
sys4 : LDLIBS+=-lmptcore
sys4 : LDFLAGS+=-Wl,--export-dynamic
test_sys4 : ARGS=sys4.conf
