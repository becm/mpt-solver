# Makefile: MPT solver operations
LIB = mptsolver
#
# additional header directories
INC = . base base/mptcore base/mptplot base/mptclient
#
# header to export
HEADER = solver.h
#
# shared library options
SHLIB_MAJOR = 1
SHLIB_MINOR = 0
SHLIB_TEENY = 0
SHLIB_OBJS := libinfo.o
LDLIBS = -lmptio -lmptplot -lmptcore -ldl
#
# source summary
SRCS := $(wildcard util/*.c) client_ivp.c client_nls.c
#
# import library creation settings
include base/mpt.lib.mk
#
# additional dependencies
libinfo.o : base/libinfo.h base/version.h release.h
#
# release information
include base/mpt.release.mk
#
# preocess solver modules
.PHONY : modules_% examples_% examples
examples : examples_all
devel  : modules_devel
clear  : modules_clear examples_clear
clean  : modules_clean examples_clean
modules_% :
	${MAKE} -C modules $(@:modules_%=%)
examples_% :
	${MAKE} -C examples $(@:examples_%=%)
