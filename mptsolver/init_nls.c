
#include <sys/uio.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief initialize NLS solver
 * 
 * Set parameter for NLS solver form data.
 * 
 * \param sol  solver descriptor
 * \param dat  solver data
 * 
 * \return pointer to nonlinear user funtions
 */
extern MPT_SOLVER_STRUCT(nlsfcn) *mpt_init_nls(MPT_SOLVER_INTERFACE *sol, const MPT_SOLVER_STRUCT(data) *dat, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { MPT_value_toVector('d'), 0 };
	
	MPT_SOLVER_STRUCT(nlsfcn) *fcns;
	struct iovec vec;
	MPT_STRUCT(property) pr;
	int32_t dim[2];
	int ret;
	
	pr.name = "";
	if ((ret = sol->_vptr->obj.property((void *) sol, &pr)) < 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unavle to get solver type"));
		return 0;
	}
	if (!(ret & MPT_SOLVER_ENUM(CapableNls))) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("con not handle  nonlinear problem"));
		return 0;
	}
	if (!(fcns = ((const MPT_INTERFACE_VPTR(Nls) *) sol->_vptr)->functions(sol))) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to get user functions"));
	}
	
	if ((dim[0] = dat->npar) < 1) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("parameter count too low"));
		return 0;
	}
	vec.iov_base = dat->param._buf + 1;
	vec.iov_len  = dim[0] * sizeof(double);
	dim[1] = 0;
	if (!dat->param._buf
	    || dim[0] > (dim[1] = dat->param._buf->used)) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s (%d < %d)",
		                 MPT_tr("not enough data for parameter count"), dim[0], dim[1]);
		return 0;
	}
	if ((dim[1] = dat->nval)
	    && dim[1] < dim[0]) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s (%d < %d)",
		                 MPT_tr("bad number of residuals for parameters"), dim[0], dim[1]);
		return 0;
	}
	pr.val.fmt = dim[1] ? "ii" : "i";
	pr.val.ptr = dim;
	if ((ret = mpt_object_pset((void *) sol, "", &pr.val, 0)) < 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s (%d, %d)",
		                 MPT_tr("failed to set problem dimensions"), dim[0], dim[1]);
		return 0;
	}
	pr.val.fmt = fmt;
	pr.val.ptr = &vec;
	if (dim[1] &&
	    (ret = mpt_object_pset((void *) sol, 0, &pr.val, 0)) < 0) {
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s (%d < %d)",
		                 MPT_tr("bad number of residuals for parameters"), dim[0], dim[1]);
		return 0;
	}
	return fcns;
}
	