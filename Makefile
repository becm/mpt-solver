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
LDLIBS = -lmptclient -lmptcore -ldl
#
# source summary
elements = client conf data solver step
SRCS := $(wildcard $(elements:%=%_*.c)) residuals_cdiff.c
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
.PHONY : modules_clean modules_shared modules_static
devel  : modules_devel
shared : modules_shared
static : modules_static ranlib
clean  : modules_clean
modules_devel modules_shared modules_static modules_clean :
	${MAKE} -C modules $(@:modules_%=%)
#
# final static library update
.PHONY : ranlib
ranlib :
	${AR} s "${LIB_FULLNAME}.a"

