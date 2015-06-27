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
SHLIB_OBJS = libinfo.o ${OBJS}
LDLIBS = -lmptio -lmptplot -lmptcore -ldl
#
# source summary
SRCS := $(wildcard util/*.c) client_ivp.c client_nls.c
#
# import library creation settings
DIR_TOP = ${MPT_PREFIX}
include base/lib.mk
# release information
include base/release.mk
#
# additional dependencies
libinfo.o : base/libinfo.h base/version.h release.h
CLEAN_FILES += libinfo.o
#
# preocess solver modules
.PHONY : modules_% examples_% examples examples_all
all : shared modules_shared
test : examples_test
examples_test : devel modules_shared
clear : modules_clear examples_clear
clean : modules_clean examples_clean
modules_% :;  ${MAKE} -C modules $(@:modules_%=%)
examples_% :; ${MAKE} -C examples $(@:examples_%=%)
