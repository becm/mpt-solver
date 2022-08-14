/*!
 * MPT solver library
 *   execute solver steps for iterator values
 */

#include <string.h>
#include <inttypes.h>

#include <math.h>
#include <sys/uio.h>

#include "output.h"
#include "convert.h"

#include "values.h"

#include "solver.h"

static int setTime(void *ptr, const MPT_STRUCT(property) *pr)
{
	if (!pr || pr->name || !pr->val.ptr) {
		return 0;
	}
	if (pr->val.type == 'd') {
		*((double *) ptr) = *((double *) pr->val.ptr);
		return pr->val.type;
	}
	if (pr->val.type == MPT_ENUM(TypeObjectPtr)) {
		MPT_STRUCT(property) pt;
		const MPT_INTERFACE(object) *obj = *((void * const *) pr->val.ptr);
		pt.name = "t";
		if (obj && obj->_vptr->property(obj, &pt) >= 0) {
			if (mpt_value_convert(&pt.val, 'd', ptr) >= 0) {
				return 0;
			}
		}
	}
	if (mpt_value_convert(&pr->val, 'd', ptr) >= 0) {
		return pr->val.type;
	}
	return MPT_ERROR(BadType);
}

static double getTime(MPT_SOLVER(interface) *sol)
{
	double t = NAN;
	sol->_vptr->report(sol, MPT_SOLVER_ENUM(Values), setTime, &t);
	return t;
}

static int updateIvpData(void *ctx, const MPT_STRUCT(value) *val)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	const MPT_INTERFACE(object) *obj;
	MPT_STRUCT(solver_data) *dat = ctx;
	double *add;
	ssize_t take;
	
	if (!(dat = ctx)) {
		return 0;
	}
	if (!val->ptr) {
		return MPT_ERROR(BadValue);
	}
	if (val->type != MPT_ENUM(TypeObjectPtr)) {
		return MPT_ERROR(BadType);
	}
	if (!(obj = *(void * const *) val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	/* add space for new data */
	take = dat->nval;
	if (!(add = mpt_values_prepare(&dat->val, take))) {
		return MPT_ERROR(BadOperation);
	}
	if (take < 0) {
		take = -take;
	}
	--take;
	take *= sizeof(*add);
	
	pr.name = "t";
	if (obj->_vptr->property(obj, &pr) >= 0) {
		if (pr.val.type == 'd' && pr.val.ptr) {
			*add = *((const double *) pr.val.ptr);
		}
	}
	pr.name = "y";
	if (obj->_vptr->property(obj, &pr) >= 0) {
		if (pr.val.type == MPT_type_toVector('d') && pr.val.ptr) {
			const struct iovec *vec = pr.val.ptr;
			ssize_t len = vec->iov_len;
			if (len > take) {
				len = take;
			}
			if (vec->iov_base) {
				memcpy(add + 1, vec->iov_base, len);
			} else {
				memset(add + 1, 0, len);
			}
		}
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
 * \param mt   IVP solver descriptor
 * \param src  time step source
 * \param out  state output output descriptor
 * \param sd   solver data
 * \param info info output descriptor
 * 
 * \return step operation result
 */
extern int mpt_solver_steps_ode(MPT_INTERFACE(convertable) *val, MPT_INTERFACE(iterator) *src, MPT_INTERFACE(logger) *out, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *info)
{
	const MPT_STRUCT(value) *t;
	MPT_SOLVER(interface) *sol;
	MPT_INTERFACE(object) *obj;
	const char *name;
	double curr, end;
	int ret;
	
	if (!src) {
		return MPT_ERROR(BadArgument);
	}
	if (!(t = src->_vptr->value(src))) {
		return MPT_ERROR(MissingData);
	}
	if ((ret = mpt_value_convert(t, 'd', &end)) < 0) {
		return ret;
	}
	obj = 0;
	if ((ret = val->_vptr->convert(val, MPT_ENUM(TypeObjectPtr), &obj)) < 0
	    || !obj) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s (%" PRIxPTR ")",
		        MPT_tr("missing object interface"), val);
		return MPT_ERROR(BadArgument);
	}
	if (!(name = mpt_object_typename(obj))) {
		name = "solver";
	}
	sol = 0;
	if ((ret = val->_vptr->convert(val, MPT_ENUM(TypeSolverPtr), &sol)) < 0
	    || !sol) {
		mpt_log(info, __func__, MPT_LOG(Error), "%s: %s (%" PRIxPTR ")",
		        name, MPT_tr("missing solver interface"), val);
		return MPT_ERROR(BadArgument);
	}
	curr = getTime(sol);
	/* try to complete full run */
	while(1) {
		mpt_log(info, __func__, MPT_LOG(Debug2), "%s: t = [%g, %g]",
		        MPT_tr("attempt solver step"), curr, end);
		
		/* set ODE/DAE solver target time */
		if ((ret = mpt_solver_setvalue(obj, "t", end)) < 0) {
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s: t = %g (told = %g)",
			        MPT_tr("failed to set target time"), end, curr);
			return ret;
		}
		ret = sol->_vptr->solve(sol);
		curr = getTime(sol);
		if (ret < 0) {
			if (out) {
				mpt_solver_status(sol, out, 0, 0);
			}
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s: t = %g (tend = %g)",
			        MPT_tr("failed solver step"), curr, end);
			return ret;
		}
		/* retry current end time */
		if (curr < end) {
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s: t = %g (tend = %g)",
			        MPT_tr("partial solver step"), curr, end);
			continue;
		}
		if (out) {
			mpt_solver_status(sol, out, updateIvpData, sd);
		} else if (sd) {
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
			if (!(t = src->_vptr->value(src))) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s",
				        MPT_tr("bad time step state"));
				return MPT_ERROR(BadOperation);
			}
			if ((ret = mpt_value_convert(t, 'd', &end)) < 0) {
				mpt_log(info, __func__, MPT_LOG(Warning), "%s: %d",
				        MPT_tr("bad time step type"), t->type);
				return ret;
			}
		} while (end <= curr);
	}
}

