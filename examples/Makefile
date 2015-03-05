# Makefile: MPT solver examples
#
DIRS  = ode dae pde nls bacol
PROGS = config
OBJS  = $(PROGS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : ${PROGS} sub_all

config : config.o
config : LDLIBS+=-lmptcore

include solver.example.mk

devel :
clear : sub_clear
clean : sub_clean
test  : sub_test
