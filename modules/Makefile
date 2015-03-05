# Makefile: create solver libraries
#
# include global configuration
include ../base/mpt.conf.mk
#
SOLIVP = vode daesolv limex bacol
SOLVER = ${SOLIVP} nlsolv nlblas sundials
#
# headers to export
DIR_INC = ${MPT_PREFIX}/include/mpt
HEADER = solver.h
#
# vecpar and share objects
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
#
# compiler options
CPPFLAGS += -I ${DIR_BASE}/mptcore
#
# general library rules
.PHONY : ${SOLVER} shared distclean header static clear_static
shared : ${SOLVER}
shared : EXVAR+=shared
clear  : clear_static

clear_static :
	${RM} "${DIR_LIB}/libmptsolver-modules.a"

static :
	@for s in ${SOLVER}; do \
		if ! ${MAKE} -C "$${s}" "${DIR_LIB}/libmptsolver-modules.a" LIB=mptsolver-modules SOL=both; then break ; fi ; \
	done
	${AR} s "${DIR_LIB}/libmptsolver-modules.a"

${OBJS} : ${HEADER}

${SOLVER} :
	@cd ${@} && ${MAKE} ${EXVAR}
#
# pass target to solver makefiles
.DEFAULT header clear :
	@for i in ${SOLVER}; do \
		if ! ${MAKE} -C "$${i}" ${@} ${EXVAR}; then break ; fi ; \
	done
