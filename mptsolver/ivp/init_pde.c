/*!
 * MPT solver library
 *   initialize PDE grid and user functions
 */

#include <inttypes.h>

#include "meta.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief initialize PDE solver
 * 
 * Set parameter for PDE solver form arguments.
 * 
 * \param sol   solver descriptor
 * \param fcn   user supplied right side function and context
 * \param neqs  number of PDE equotations
 * \param arr   grid data points
 * \param info  log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern int mpt_init_pde(const MPT_INTERFACE(metatype) *mt, const MPT_IVP_STRUCT(pdefcn) *fcn, int neqs, const _MPT_ARRAY_TYPE(double) *arr, MPT_INTERFACE(logger) *info)
{
	MPT_INTERFACE(object) *obj;
	MPT_SOLVER(interface) *sol;
	MPT_STRUCT(buffer) *buf;
	uint64_t len;
	int ret, types;
	
	if (fcn && !fcn->fcn) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing right side function"));
		}
		return MPT_ERROR(BadArgument);
	}
	if (neqs <= 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("bad equotation count"));
		}
		return MPT_ERROR(BadValue);
	}
	sol = 0;
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol)) < 0
	    || !sol) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get solver interface"), mt);
		}
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj)) < 0
	    || !obj) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get object interface"), mt);
		}
		return MPT_ERROR(BadType);
	}
	len = 0;
	if (!arr) {
		if ((ret = mpt_object_set(obj, "", "i", neqs)) < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s (%d)",
				        MPT_tr("failed to set PDE count"), neqs);
			}
			return ret;
		}
	}
	else if (!(buf = arr->_buf)
	         || (len = buf->_used / sizeof(double)) < 2) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s: %" PRIu64,
			        MPT_tr("bad grid size"), len);
		}
		return MPT_ERROR(BadValue);
	}
	else {
		static const char fmt[] = { 'i', MPT_value_toVector('d'), 0 };
		struct iovec vec;
		
		vec.iov_base = buf + 1;
		vec.iov_len  = len * sizeof(double);
		if ((ret = mpt_object_set(obj, "", fmt, neqs, vec)) < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s: %d, [%" PRIu64 "]",
				        MPT_tr("failed to set PDE parameters"), neqs, len);
			}
			return ret;
		}
	}
	if (!fcn) {
		return 0;
	}
	types = MPT_SOLVER_ENUM(IvpRside) | MPT_SOLVER_ENUM(PDE);
	if ((ret = sol->_vptr->set_functions(sol, types, fcn)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("unable to set PDE user functions"));
		}
	}
	else if ((ret = sol->_vptr->set_functions(sol, ~types, 0)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("unable to set PDE user functions"));
		}
	}
	return ret;
}
