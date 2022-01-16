/*!
 * MPT solver library
 *   set initial ODE/DAE values
 */

#include <sys/uio.h>

#include "types.h"
#include "array.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief initialize IVP data
 * 
 * Set initial values for ODE and DAE problems
 * for IVP solver from array data.
 * 
 * \param sol   solver object
 * \param arr   array with double values (time and state)
 * \param info  log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern int mpt_ivp_data(MPT_INTERFACE(object) *sol, const _MPT_ARRAY_TYPE(double) *arr, MPT_INTERFACE(logger) *info)
{
	static const char fmt[] = { 'd', MPT_type_toVector('d'), 0 };
	
	const MPT_STRUCT(type_traits) *traits;
	const MPT_STRUCT(buffer) *buf;
	double t;
	struct iovec y;
	double *src;
	size_t len;
	int ret;
	
	if (!sol) {
		return MPT_ERROR(BadArgument);
	}
	if (!arr
	    || !(buf = arr->_buf)
	    || !(len = buf->_used / sizeof(*src))) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing initial value data"));
		}
		return MPT_ERROR(BadValue);
	}
	if (!(traits = buf->_content_traits)
	    || traits != mpt_type_traits('d')) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s ('d')",
		        MPT_tr("bad initial value data"));
		return MPT_ERROR(BadType);
	}
	/* initial value setup */
	src = (void *) (buf + 1);
	t = *src;
	y.iov_base = src + 1;
	y.iov_len  = (len - 1) * sizeof(*src);
	
	if ((ret = mpt_object_set(sol, 0, fmt, t, y)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("failed to set initial values"));
		}
	}
	return len;
}
