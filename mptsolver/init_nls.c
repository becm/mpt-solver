
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
extern int mpt_init_nls(MPT_SOLVER(interface) *sol, const MPT_NLS_STRUCT(functions) *fcn, const MPT_STRUCT(solver_data) *dat, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { MPT_value_toVector('d'), 0 };
	struct iovec vec;
	int32_t npar, nres;
	int ret;
	
	if (fcn && !fcn->res.fcn) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("missing residual function pointer"));
		return MPT_ERROR(BadArgument);
	}
	if ((npar = dat->npar) < 1) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("parameter count too low"));
		return MPT_ERROR(BadArgument);
	}
	vec.iov_base = dat->param._buf + 1;
	vec.iov_len  = npar * sizeof(double);
	nres = dat->param._buf ? dat->param._buf->_used / sizeof(double) : 0;
	if (npar > nres) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (%d < %d)",
		                 MPT_tr("not enough data for parameter count"), nres, npar);
		return MPT_ERROR(BadValue);
	}
	if ((nres = dat->nval)
	    && nres < npar) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (%d < %d)",
		                 MPT_tr("bad number of residuals for parameters"), nres, npar);
		return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_object_set((void *) sol, "", nres ? "ii" : "i", npar, nres)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (npar = %d, nres = %d)",
		                 MPT_tr("failed to set problem dimensions"), npar, nres);
		return ret;
	}
	if (fcn && (ret = sol->_vptr->setFunctions(sol, MPT_SOLVER_ENUM(NlsUser), fcn)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to set user functions"));
		return ret;
	}
	if ((ret = mpt_object_set((void *) sol, 0, fmt, vec)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s (%d)",
		                 MPT_tr("failed to set initial parameters"), npar);
		return ret;
	}
	return 0;
}
