
#include <inttypes.h>
#include <sys/uio.h>

#include "meta.h"
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
 * \param info  log/error output descriptor
 * 
 * \return pointer to nonlinear user funtions
 */
extern int mpt_init_ivp(MPT_INTERFACE(object) *sol, const _MPT_ARRAY_TYPE(double) *arr, MPT_INTERFACE(logger) *info)
{
	static const uint8_t fmt[] = { 'd', MPT_value_toVector('d'), 0 };
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
	if (!arr
	    || !(buf = arr->_buf)
	    || !(len = buf->_used / sizeof(*src))) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing initial value data"));
		}
		return MPT_ERROR(BadValue);
	}
	if ((ret = buf->_vptr->content(buf))
	    && ret != 'd') {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s ('%d' != 'd')",
			        MPT_tr("missing initial value data"), ret);
		}
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
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("failed to set initial values"));
		}
	}
	return len;
}

static int initIvpData(MPT_SOLVER(interface) *sol, const void *fcn, int neqs, MPT_INTERFACE(logger) *info, MPT_INTERFACE(object) *obj, int type, const char *_func)
{
	int ret;
	
	/* set equotation count */
	if (obj &&
	    (ret = mpt_object_set(obj, "", "i", (int32_t) neqs)) < 0) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("unable to save problem parameter to solver"));
		}
		return ret;
	}
	/* set user funtions according to type */
	if (!fcn) {
		type = 0;
	}
	else if ((ret = sol->_vptr->setFunctions(sol, type, fcn)) < 0) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (0x%x)",
			        MPT_tr("unable to set user functions"), type);
		}
		return ret;
	}
	/* limit user functions to compatible types */
	if ((ret = sol->_vptr->setFunctions(sol, ~type, 0)) < 0) {
		if (info) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (0x%x)",
			        MPT_tr("unable to limit user functions"), type);
		}
	}
	return ret;
}

extern int mpt_init_dae(const MPT_INTERFACE(metatype) *mt, const MPT_IVP_STRUCT(daefcn) *fcn, int neqs, MPT_INTERFACE(logger) *info)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	
	if (fcn && !fcn->rside.fcn) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing user right side"));
		}
		return MPT_ERROR(BadArgument);
	}
	sol = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0
	    || !sol) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get solver interface"), mt);
		}
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
	    && !obj) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("no object interface for solver"));
		}
		return MPT_ERROR(BadValue);
	}
	return initIvpData(sol, fcn, neqs, info, obj, MPT_SOLVER_ENUM(DAE), __func__);
}
extern int mpt_init_ode(const MPT_INTERFACE(metatype) *mt, const MPT_IVP_STRUCT(odefcn) *fcn, int neqs, MPT_INTERFACE(logger) *info)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	
	if (fcn && !fcn->rside.fcn) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("missing user right side"));
		}
		return MPT_ERROR(BadArgument);
	}
	sol = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol) < 0
	    || !sol) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
			        MPT_tr("failed to get solver interface"), mt);
		}
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
	    && !obj) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("no object interface for solver"));
		}
		return MPT_ERROR(BadValue);
	}
	return initIvpData(sol, fcn, neqs, info, obj, MPT_SOLVER_ENUM(ODE), __func__);
}
