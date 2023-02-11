# Makefile: DAE client programs
#
PROGS = robertson cakzo
OBJS  = $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : ${EXAMPLES_BASE}/solver_run.o

$(PROGS:%=%.o) : ${EXAMPLES_BASE}/solver_run.h

cakzo : cakzo.o
cakzo cakzo_static : LDLIBS+=-lm
test_cakzo : ARGS=cakzo.conf

robertson : robertson.o
test_robertson : ARGS=robertson.conf

test_% : MPT_FLAGS=e
