
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
extern int mpt_init_pde(MPT_SOLVER(generic) *sol, const MPT_SOLVER_IVP_STRUCT(pdefcn) *fcn, int neqs, const _MPT_ARRAY_TYPE(double) *arr, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { 'i', MPT_value_toVector('d'), 0 };
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(value) val;
	struct iovec vec;
	int32_t *neq;
	uint8_t tmp[sizeof(*neq) + sizeof(vec)];
	uint64_t len;
	int ret;
	
	if (!fcn || !fcn->fcn) {
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
	/* aligned value content */
	neq = (void *) tmp;
	*neq = neqs;
	memcpy(neq + 1, &vec, sizeof(vec));
	val.fmt = fmt;
	val.ptr = neq;
	if ((ret = mpt_object_nset((void *) sol, "", &val)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s [%d, " PRIu64 "]",
		                 MPT_tr("failed to set PDE size"), neqs, len);
		return ret;
	}
	if ((ret = sol->_vptr->setFunctions(sol, MPT_SOLVER_ENUM(IvpRside) | MPT_SOLVER_ENUM(PDE), fcn)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to get PDE user functions"));
	}
	return ret;
}
