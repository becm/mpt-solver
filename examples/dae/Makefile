# Makefile: DAE client programs
#
PROGS = robertson cakzo
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${DIR_EXAMPLES}solver_run.o

${DIR_EXAMPLES}*.o *.o : ../solver_run.h

cakzo : cakzo.o
cakzo cakzo_static : LDLIBS+=-lm
test_cakzo : ARGS=cakzo.conf

robertson : robertson.o
test_robertson : ARGS=robertson.conf
