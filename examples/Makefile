# Makefile: MPT solver examples
#
DIRS = ode dae pde nls bacol
PROGS = config
OBJS = $(PROGS:%=%.o)

# .PHONY : all devel clear clean static test distclean
all : sub ${PROGS}
sub :
	@for d in ${DIRS}; do \
		if ! ${MAKE} -C "$${d}" ${TARGET}; then break; fi; \
	done

config : config.o

include solver.example.mk

devel :
clear : subclear
clean : subclean
test  : subtest

# cleansub : clearsub
subclean subclear subtest :
	@for d in ${DIRS}; do ${MAKE} -C "$${d}" $(@:sub%=%); done;
