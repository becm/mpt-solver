# Makefile: PDE client programs
#
PROGS = flame diff htrans twoeq makzo
OBJS  = main.o $(PROGS:%=%.o)

# general targets and definitions
include ../solver.example.mk

${PROGS} ${STATIC} : main.o

flame : flame.o
flame flame_static : LDLIBS+=-lm
test_flame : ARGS=flame.conf

diff : diff.o
test_diff : ARGS=twoeq.conf

htrans : htrans.o
test_htrans : ARGS=twoeq.conf

twoeq : twoeq.o
twoeq : LDLIBS+=-lmptcore
twoeq twoeq_static : LDLIBS+=-lm
test_twoeq : ARGS=twoeq.conf

makzo : makzo.o
test_makzo : ARGS=makzo.conf
