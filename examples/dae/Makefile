# Makefile: DAE client programs
#
PROGS = robertson cakzo
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ../main_ivp.o ../solver_run.o

cakzo : cakzo.o
cakzo cakzo_static : LDLIBS+=-lm
test_cakzo : ARGS=cakzo.conf

robertson : robertson.o
test_robertson : ARGS=robertson.conf
