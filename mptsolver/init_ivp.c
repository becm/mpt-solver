
#include <sys/uio.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief initialize IVP state
 * 
 * Set initial values for ODE and DAE problems
 * for IVP solver form arguments.
 * 
 * \param sol   solver object
 * \param t     initial time
 * \param len   number of equotations
 * \param ptr   initial state data
 * \param log   log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern int mpt_init_ivp(MPT_INTERFACE(object) *sol, const _MPT_ARRAY_TYPE(double) *arr, MPT_INTERFACE(logger) *log)
{
	static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
	const MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(value) val;
	struct {
		double t;
		struct iovec y;
	} data;
	double *src;
	size_t len;
	int ret;
	
	if (!sol) {
		return MPT_ERROR(BadArgument);
	}
	if (!arr || !(buf = arr->_buf) || !(len = buf->_used / sizeof(*src))) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("missing initial value data"));
		return MPT_ERROR(BadValue);
	}
	if ((ret = buf->_vptr->content(buf)) && ret != 'd') {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s ('%d' != 'd')",
		                 MPT_tr("missing initial value data"), ret);
		return MPT_ERROR(BadType);
	}
	/* initial value setup */
	src = (void *) (buf + 1);
	data.t = *src;
	data.y.iov_base = src + 1;
	data.y.iov_len  = (len - 1) * sizeof(double);
	val.fmt = fmt;
	val.ptr = &data;
	
	if ((ret = mpt_object_set_value(sol, 0, &val)) < 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("failed to set initial values"));
	}
	return len;
}

static int initIvpData(MPT_SOLVER(interface) *sol, const void *fcn, int neqs, MPT_INTERFACE(logger) *log, int type, const char *_func)
{
	MPT_INTERFACE(object) *obj;
	int ret;
	
	/* use object for initial setup */
	obj = 0;
	if ((ret = sol->_vptr->meta.conv((void *) sol, MPT_ENUM(TypeObject), &obj)) >= 0) {
		if (!obj) {
			if (log) {
				mpt_log(log, _func, MPT_LOG(Error), "%s",
				        MPT_tr("no object interface for solver"));
			}
			return MPT_ERROR(BadValue);
		}
		/* set equotation count */
		if ((ret = mpt_object_set(obj, "", "i", (int32_t) neqs)) < 0) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
			                 MPT_tr("unable to save problem parameter to solver"));
			return ret;
		}
	}
	/* set user funtions according to type */
	if (fcn && (ret = sol->_vptr->setFunctions(sol, type, fcn)) < 0) {
		if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to set user functions"));
	}
	return ret;
}

extern int mpt_init_dae(MPT_SOLVER(interface) *sol, const MPT_IVP_STRUCT(daefcn) *fcn, int neqs, MPT_INTERFACE(logger) *log)
{
	if (fcn && !fcn->rside.fcn) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("missing user right side"));
	}
	return initIvpData(sol, fcn, neqs, log, MPT_SOLVER_ENUM(DAE), __func__);
}
extern int mpt_init_ode(MPT_SOLVER(interface) *sol, const MPT_IVP_STRUCT(odefcn) *fcn, int neqs, MPT_INTERFACE(logger) *log)
{
	if (fcn && !fcn->rside.fcn) {
		if (log) mpt_log(log, __func__, MPT_LOG(Error), "%s",
		                 MPT_tr("missing user right side"));
	}
	return initIvpData(sol, fcn, neqs, log, MPT_SOLVER_ENUM(ODE), __func__);
}
