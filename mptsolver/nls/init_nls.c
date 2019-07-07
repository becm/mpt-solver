/*!
 * MPT solver library
 *   set nonlinear solver initial data and user functions
 */

#include <sys/uio.h>
#include <inttypes.h>

#include "meta.h"
#include "array.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief initialize NLS solver
 * 
 * Set parameter for NLS solver form data.
 * 
 * \param val   nonlinear solver dispatcher
 * \param fcn   NLS user functions 
 * \param dat   solver data 
 * \param info  log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern int mpt_init_nls(MPT_INTERFACE(convertable) *val, const MPT_NLS_STRUCT(functions) *fcn, const MPT_STRUCT(solver_data) *dat, MPT_INTERFACE(logger) *info)
{
	static const char fmt[] = { MPT_type_vector('d'), 0 };
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	struct iovec vec;
	int type = MPT_SOLVER_ENUM(NlsUser) | MPT_SOLVER_ENUM(NlsJac);
	int32_t npar, nres;
	int ret;
	
	if (fcn && !fcn->res.fcn) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing residual function pointer"));
		}
		return MPT_ERROR(BadArgument);
	}
	sol = 0;
	if ((ret = val->_vptr->convert(val, MPT_type_pointer(MPT_ENUM(TypeSolver)), &sol)) < 0
	    || !sol) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get solver interface"), val);
		}
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if ((ret = val->_vptr->convert(val, MPT_type_pointer(MPT_ENUM(TypeObject)), &obj)) < 0
	    || !obj) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("solver without object interface"), val);
		}
		return MPT_ERROR(BadArgument);
	}
	if ((npar = dat->npar) < 1) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %d",
			        MPT_tr("parameter count too low"), npar);
		}
		return MPT_ERROR(BadArgument);
	}
	nres = dat->param._buf ? dat->param._buf->_used / sizeof(double) : 0;
	if (npar > nres) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%d < %d)",
			        MPT_tr("not enough data for parameter count"), nres, npar);
		}
		return MPT_ERROR(BadValue);
	}
	if ((nres = dat->nval)
	    && nres < npar) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%d < %d)",
			        MPT_tr("bad number of residuals for parameters"), nres, npar);
		}
		return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_object_set(obj, "", nres ? "ii" : "i", npar, nres)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (npar = %d, nres = %d)",
			        MPT_tr("failed to set problem dimensions"), npar, nres);
		}
		return ret;
	}
	if (!fcn) {
		type = 0;
	}
	else if ((ret = sol->_vptr->set_functions(sol, type, fcn)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("unable to set user functions"));
		}
		return ret;
	}
	if ((ret = sol->_vptr->set_functions(sol, ~type, 0)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("unable to set user functions"));
		}
		return ret;
	}
	vec.iov_base = dat->param._buf + 1;
	vec.iov_len  = npar * sizeof(double);
	if ((ret = mpt_object_set(obj, 0, fmt, vec)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%d)",
			        MPT_tr("failed to set initial parameters"), npar);
		}
		return ret;
	}
	return 0;
}
