# Makefile: MPT bacol examples
#
PROGS = sys4 bacol
OBJS  = main.o bacolWrap.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

# equotations, wrapper settings
${PROGS} : sys4.o

# PDE client main
sys4 : LDFLAGS += -Wl,--export-dynamic
sys4 : LDLIBS += -lmptsolver -lmptclient -lmptplot -lmptcore
sys4 : main.o
