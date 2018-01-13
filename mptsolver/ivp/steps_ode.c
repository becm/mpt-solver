
#include <string.h>
#include <inttypes.h>

#include <sys/uio.h>

#include "array.h"
#include "meta.h"

#include "values.h"

#include "solver.h"

static int setTime(void *ptr, const MPT_STRUCT(property) *pr)
{
	if (!pr || pr->name) {
		return 0;
	}
	if (!pr->val.fmt || *pr->val.fmt != 'd') {
		return MPT_ERROR(BadValue);
	}
	*((double *) ptr) = *((double *) pr->val.ptr);
	return 1;
}

static double getTime(MPT_SOLVER(interface) *sol)
{
	double t;
	sol->_vptr->report(sol, MPT_SOLVER_ENUM(Values), setTime, &t);
	return t;
}

static int updateIvpData(void *ctx, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(solver_data) *dat = ctx;
	struct iovec *vec;
	const double *t;
	double *add;
	size_t take, len;
	
	if (!val->fmt
	    || val->fmt[0] != 'd'
	    || val->fmt[1] != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (!(t = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	/* add space for new data */
	take = dat->nval;
	if (!(add = mpt_values_prepare(&dat->val, take--))) {
		return MPT_ERROR(BadOperation);
	}
	take *= sizeof(*add);
	
	/* limit accepted size */
	vec = (void *) (t + 1);
	if ((len = vec->iov_len) > take) {
		len = take;
	}
	/* copy current state */
	*add = *t;
	if ((t = vec->iov_base)) {
		memcpy(add+1, t, len);
	}
	return 2;
}
static int updateIvpDataWrap(void *ctx, const MPT_STRUCT(property) *pr)
{
	if (!pr || pr->name) {
		return 0;
	}
	return updateIvpData(ctx, &pr->val);
}

/*!
 * \ingroup mptSolver
 * \brief ODE steps
 * 
 * Execute generic DAE/ODE solver steps.
 * 
 * \param sol  IVP solver descriptor
 * \param src  time step source
 * \param sd   solver data
 * \param info log/error output descriptor
 * 
 * \return step operation result
 */
extern int mpt_steps_ode(MPT_INTERFACE(metatype) *mt, MPT_INTERFACE(iterator) *src, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *info)
{
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	const char *name;
	double curr, end, *data;
	int ret;
	
	if (!sd || !src) {
		return MPT_ERROR(BadArgument);
	}
	if ((ret = src->_vptr->get(src, 'd', &end)) < 0) {
		return MPT_ERROR(MissingData);
	}
	if (!ret) {
		return 0;
	}
	if (sd->nval < 1 || !(data = mpt_solver_data_grid(sd))) {
		return MPT_ERROR(BadArgument);
	}
	obj = 0;
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj)) < 0
	    || !obj) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
		        MPT_tr("missing object interface"), mt);
		return MPT_ERROR(BadArgument);
	}
	if (!(name = mpt_object_typename(obj))) {
		name = "solver";
	}
	sol = 0;
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeSolver), &sol)) < 0
	    || !sol) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s: %s (%" PRIxPTR ")",
		        name, MPT_tr("missing solver interface"), mt);
		return MPT_ERROR(BadArgument);
	}
	curr = getTime(sol);
	/* try to complete full run */
	while(1) {
		mpt_log(info, __func__, MPT_LOG(Debug2), "%s (t = %g > %g)",
		        MPT_tr("attempt solver step"), curr, end);
		
		/* set ODE/DAE solver target time */
		if ((ret = mpt_solver_setvalue(obj, "t", end)) < 0) {
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s (t = %g > %g)",
			        MPT_tr("failed to set target time"), curr, end);
			return ret;
		}
		ret = sol->_vptr->solve(sol);
		curr = getTime(sol);
		if (ret < 0) {
			if (info) {
				mpt_solver_status(sol, info, 0, 0);
			}
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s (t = %g > %g)",
			        MPT_tr("failed solver step"), curr, end);
			return ret;
		}
		/* retry current end time */
		if (curr < end) {
			curr = end;
			continue;
		}
		if (info) {
			mpt_solver_status(sol, info, updateIvpData, sd);
		} else {
			sol->_vptr->report(sol, MPT_SOLVER_ENUM(Values), updateIvpDataWrap, sd);
		}
		/* get next target value */
		do {
			if ((ret = src->_vptr->advance(src)) < 0) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s",
				        MPT_tr("bad time step advance"));
				return ret;
			}
			if (!ret) {
				return ret;
			}
			if ((ret = src->_vptr->get(src, 'd', &end)) < 0) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s",
				        MPT_tr("bad time step data"));
				return ret;
			}
			if (!ret) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("bad time step state"));
				return ret;
			}
		} while (end <= curr);
	}
}
