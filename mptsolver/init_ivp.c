
#include <sys/uio.h>

#include "array.h"

#include "solver.h"

extern int mpt_init_ivp(MPT_SOLVER(IVP) *sol, double t, int len, const double *ptr, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
	
	struct {
		double t;
		struct iovec val;
	} tmp;
	MPT_STRUCT(value) val;
	int32_t neqs;
	int ret;
	
	if ((neqs = len) < 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to save problem parameter to solver"));
		return MPT_ERROR(BadArgument);
	}
	/* set equotation count */
	val.fmt = "i";
	val.ptr = &neqs;
	if ((ret = mpt_object_pset((void *) sol, "", &val, 0)) <= 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to save problem parameter to solver"));
		return ret;
	}
	/* initial value setup */
	tmp.t = t;
	tmp.val.iov_base = (void *) ptr;
	tmp.val.iov_len  = len * sizeof(double);
	
	val.fmt = fmt;
	val.ptr = &tmp;
	
	if ((ret = mpt_object_pset((void *) sol, 0, &val, 0)) < 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("failed to set initial values"));
	}
	return ret;
}

static void *initIvpData(MPT_SOLVER(IVP) *sol, const MPT_STRUCT(array) *val, MPT_INTERFACE(logger) *log, int type, const char *_func)
{
	const MPT_STRUCT(buffer) *buf;
	const double *ptr;
	void *fcn;
	int ret;
	
	if (!(buf = val->_buf)
	    || !(ret = buf->used / sizeof(double))) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("missing initial data"));
		return 0;
	}
	ptr = (double *) (buf + 1);
	if (mpt_init_ivp(sol, *ptr, ret - 1, ptr+1, log) < 0) {
		return 0;
	}
	/* get user funtions according to type */
	if (!(fcn = sol->_vptr->functions(sol, type))) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to get user functions"));
	}
	return fcn;
}

extern MPT_SOLVER_STRUCT(daefcn) *mpt_init_dae(MPT_SOLVER(IVP) *sol, const MPT_STRUCT(array) *val, MPT_INTERFACE(logger) *log)
{
	return initIvpData(sol, val, log, MPT_SOLVER_ENUM(DAE), __func__);
}
extern MPT_SOLVER_STRUCT(odefcn) *mpt_init_ode(MPT_SOLVER(IVP) *sol, const MPT_STRUCT(array) *val, MPT_INTERFACE(logger) *log)
{
	return initIvpData(sol, val, log, MPT_SOLVER_ENUM(ODE), __func__);
}
