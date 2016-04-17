
#include "solver.h"

extern MPT_SOLVER_STRUCT(pdefcn) *mpt_init_pde(MPT_SOLVER(IVP) *sol, int neqs, int len, MPT_INTERFACE(logger) *log)
{
	int32_t ivp[2];
	MPT_STRUCT(value) val;
	void *fcn;
	
	if (neqs <= 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("bad equotation count"));
		return 0;
	}
	if (len < 2) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("bad grid size"));
		return 0;
	}
	
	ivp[0] = neqs;
	ivp[1] = len - 1;
	
	val.fmt = "ii";
	val.ptr = ivp;
	
	if (mpt_object_pset((void *) sol, "", &val, 0) < 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s [%d, %d]",
		                 MPT_tr("failed to set PDE size"), neqs, len);
		return 0;
	}
	if (!(fcn = sol->_vptr->functions(sol, MPT_SOLVER_ENUM(PDE)))) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to get PDE user functions"));
	}
	return fcn;
}
