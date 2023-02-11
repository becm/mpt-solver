# Makefile: create solver libraries
#
# solver collection library
LIB = mpt_solvers
#
# include global configuration
include ../base/mpt.config.mk
#
SOLIVP = vode daesolv limex bacol
SOLVER = ${SOLIVP} nlsolv nlblas sundials
#
# general library rules
.PHONY : static clear_static
install :
clear : clear_static
clean : clean_objects

clean_objects :
	${RM} mod_*.o

clear_static :
	${RM} "${DIR_LIB}/lib${LIB}.a"

static :
	@for s in ${SOLVER}; do \
		if ! ${MAKE} -C "$${s}" "${DIR_LIB}/lib${LIB}.a" LIB=${LIB}; then break ; fi ; \
	done
	${AR} s "${DIR_LIB}/lib${LIB}.a"
#
# pass target to solver makefiles
.PHONY : install header clear clean
.DEFAULT install header clear clean :
	@for i in ${SOLVER}; do \
		if ! ${MAKE} -C "$${i}" ${@}; then break ; fi ; \
	done
#
