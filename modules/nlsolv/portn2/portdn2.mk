#portdn2.mk
# routines for nonlinear solver family 'N2' and dependancies
# from port library for double precision.

# objects for unbounded solvers
portdn2fgp_obj = dn2f.o dn2g.o dn2p.o
portdn2_unbound = \
	drn2g.o dn2rdp.o \
	dg7lit.o dn2cvp.o dc7vfn.o dn2lrd.o df7hes.o \
	dl7svx.o dl7nvr.o dl7tsq.o do7prd.o 

# objects for bounded solvers
portdn2fgpb_obj = dn2fb.o dn2gb.o dn2pb.o
portdn2_bound = \
	drn2gb.o \
	dg7itb.o dr7tvm.o \
	df7dhb.o i7pnvr.o dl7msb.o dg7qsb.o i7copy.o ds7ipr.o \
	ds7dmp.o i7shft.o dq7rsh.o dv7vmp.o dv7ipr.o \
	dd7mlp.o ds7bqn.o dh2rfg.o dh2rfa.o dv7shf.o

# shared objects for all solvers
portdn2_shared = \
	divset.o \
	ditsum.o dd7tpr.o dv2nrm.o dv7scp.o dq7rad.o dd7upd.o \
	dl7vml.o dq7apl.o dv7cpy.o dv7dfl.o i7mdcn.o \
	dparck.o drldst.o da7sst.o ds7lvm.o dv2axy.o \
	stopx.o  ds7lup.o dl7tvm.o dl7sqr.o dv7scl.o i1mach.o \
	dg7qts.o dl7ivm.o dl7itv.o dl7srt.o dr7mdc.o dl7mst.o \
	dl7svn.o d1mach.o

# FORTRAN code form external directory
portdn2_math = \
	$(portdn2fgp_obj:%=%) \
	$(portdn2fgpb_obj:%=%) \
	$(portdn2_unbound:%=%) \
	$(portdn2_bound:%=%) \
	$(portdn2_shared:%=%)

# C wrapper code
portn2_wrapper = \
	portn2_finit.o \
	portn2_create.o \
	portn2_prepare.o \
	portn2_property.o \
	portn2_report.o \
	portn2_solve.o \
	portn2_ufcn.o \
	portn2_residuals.o

