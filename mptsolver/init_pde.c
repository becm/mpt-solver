
#include <inttypes.h>
#include <string.h>

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
 * \param log   log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern int mpt_init_pde(MPT_SOLVER(generic) *sol, const MPT_IVP_STRUCT(pdefcn) *fcn, int neqs, const _MPT_ARRAY_TYPE(double) *arr, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { 'i', MPT_value_toVector('d'), 0 };
	MPT_STRUCT(buffer) *buf;
	struct iovec vec;
	uint64_t len;
	int ret;
	
	if (fcn && !fcn->fcn) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("missing right side function"));
		return MPT_ERROR(BadArgument);
	}
	if (neqs <= 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("bad equotation count"));
		return MPT_ERROR(BadValue);
	}
	if (!arr
	 || !(buf = arr->_buf)
	 || (len = buf->_used / sizeof(double)) < 2) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s: " PRIu64,
		                 MPT_tr("bad grid size"), len);
		return MPT_ERROR(BadValue);
	}
	vec.iov_base = buf + 1;
	vec.iov_len  = len * sizeof(double);
	if ((ret = mpt_object_set((void *) sol, "", fmt, neqs, vec)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s [%d, " PRIu64 "]",
		                 MPT_tr("failed to set PDE size"), neqs, len);
		return ret;
	}
	if (fcn && (ret = sol->_vptr->setFunctions(sol, MPT_SOLVER_ENUM(IvpRside) | MPT_SOLVER_ENUM(PDE), fcn)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to get PDE user functions"));
	}
	return ret;
}
