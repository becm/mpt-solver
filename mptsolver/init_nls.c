
#include <sys/uio.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief initialize NLS solver
 * 
 * Set parameter for NLS solver form data.
 * 
 * \param sol  nonlinear solver descriptor
 * \param dat  solver data 
 * \param log  log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern MPT_SOLVER_NLS_STRUCT(functions) *mpt_init_nls(MPT_SOLVER(NLS) *sol, const MPT_STRUCT(solver_data) *dat, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { MPT_value_toVector('d'), 0 };
	
	MPT_SOLVER_NLS_STRUCT(functions) *fcns;
	struct iovec vec;
	MPT_STRUCT(value) val;
	int32_t dim[2];
	int ret;
	
	if (!(fcns = sol->_vptr->functions(sol))) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to get user functions"));
	}
	
	if ((dim[0] = dat->npar) < 1) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("parameter count too low"));
		return 0;
	}
	vec.iov_base = dat->param._buf + 1;
	vec.iov_len  = dim[0] * sizeof(double);
	dim[1] = 0;
	if (!dat->param._buf
	    || dim[0] > (dim[1] = dat->param._buf->_used)) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (%d < %d)",
		                 MPT_tr("not enough data for parameter count"), dim[0], dim[1]);
		return 0;
	}
	if ((dim[1] = dat->nval)
	    && dim[1] < dim[0]) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (%d < %d)",
		                 MPT_tr("bad number of residuals for parameters"), dim[0], dim[1]);
		return 0;
	}
	val.fmt = dim[1] ? "ii" : "i";
	val.ptr = dim;
	if ((ret = mpt_object_iset((void *) sol, "", &val)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (npar = %d, nres = %d)",
		                 MPT_tr("failed to set problem dimensions"), dim[0], dim[1]);
		return 0;
	}
	val.fmt = fmt;
	val.ptr = &vec;
	if ((ret = mpt_object_iset((void *) sol, 0, &val)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
		                 MPT_tr("failed to set initial parameters"), dim[0]);
		return 0;
	}
	return fcns;
}
