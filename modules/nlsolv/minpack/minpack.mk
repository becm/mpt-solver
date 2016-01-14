#minpack.mk
# routines for MINPACK

# hybr[jd] solver objects
hybrid_objects = \
	hybrd1.o hybrd.o hybrj1.o hybrj.o \
	fdjac1.o r1mpyq.o r1updt.o qform.o dogleg.o

# lm* solver objects
lmderv_objects = \
	lmder1.o lmder.o lmdif1.o lmdif.o lmstr1.o lmstr.o lmpar.o \
	fdjac2.o rwupdt.o qrsolv.o

# shared objects for all solvers
minpack_shared = dpmpar.o qrfac.o enorm.o

# FORTRAN code form external directory
minpack_math = \
	$(hybrid_objects:%=hybrid/%) \
	$(lmderv_objects:%=lmderv/%) \
	$(minpack_shared:%=%) \
	chkder.o

# C wrapper code
minpack_wrapper = \
	minpack_finit.o \
	minpack_create.o \
	minpack_prepare.o \
	minpack_property.o \
	minpack_solve.o \
	minpack_report.o \
	minpack_ufcn_hybrid.o \
	minpack_ufcn_lmderv.o
