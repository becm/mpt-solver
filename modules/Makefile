# Makefile: create solver libraries
#
# solver collection library
LIB = mpt_solvers
#
# include global configuration
DIR_TOP = ${MPT_PREFIX}
DIR_INC = ${MPT_PREFIX}/include/mpt
DIR_BASE ?= ../base/
include ${DIR_BASE}mpt.config.mk
#
SOLIVP = vode daesolv limex bacol
SOLVER = ${SOLIVP} nlsolv nlblas sundials
#
# headers to export
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
.PHONY : static clear_static
install :
clear : clear_static

clear_static :
	${RM} "${DIR_LIB}/lib${LIB}.a"

static :
	@for s in ${SOLVER}; do \
		if ! ${MAKE} -C "$${s}" "${DIR_LIB}/lib${LIB}.a" LIB=${LIB} SOL=both; then break ; fi ; \
	done
	${AR} s "${DIR_LIB}/lib${LIB}.a"

${OBJS} : ${HEADER}
#
# pass target to solver makefiles
.PHONY : install header clear
.DEFAULT install header clear :
	@for i in ${SOLVER}; do \
		if ! ${MAKE} -C "$${i}" ${@}; then break ; fi ; \
	done
#
