
#include <string.h>

#include <sys/uio.h>

#include "array.h"
#include "meta.h"

#include "values.h"

#include "solver.h"

static int updateIvpData(void *ctx, const MPT_STRUCT(value) *val)
{
	MPT_SOLVER_STRUCT(data) *dat = ctx;
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
 * \param md   client data
 * \param out  logging descriptor
 * 
 * \return step operation result
 */
extern int mpt_steps_ode(MPT_SOLVER(IVP) *sol, MPT_INTERFACE(metatype) *src, MPT_SOLVER_STRUCT(data) *md, MPT_INTERFACE(logger) *out)
{
	double curr, end, *data;
	int err, cont, neqs;
	
	if (!md || !src) {
		return MPT_ERROR(BadArgument);
	}
	if ((cont = src->_vptr->conv(src, 'd', &end)) < 0) {
		return cont;
	}
	if (!cont) {
		return MPT_ERROR(MissingData);
	}
	if ((neqs = md->nval) < 1 || !(data = mpt_data_grid(md))) {
		return MPT_ERROR(BadArgument);
	}
	curr = *data;
	/* try to complete full run */
	do {
	
		if (out) mpt_log(out, __func__, MPT_LOG(Debug2), "%s (t = %g > %g)",
		                 MPT_tr("attempt solver step"), curr, end);
		
		/* call ODE/DAE solver with current/target time and in/out-data */
		curr = end;
		err = sol->_vptr->step(sol, &curr);
		
		/* retry current end time */
		if (curr < end) {
			if (out) mpt_solver_status((void *) sol, out, 0, 0);
			curr = end;
			cont = 1;
			if (err < 0) {
				break;
			}
			continue;
		}
		/* get next target value */
		if (err >= 0) {
			err = 1;
			do {
				double tmp;
				if ((cont = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &tmp)) <= 0
				    || (cont = src->_vptr->conv(src, 'd', &end)) <= 0) {
					if (!cont) err = 0;
					break;
				}
			} while (end <= curr);
		} else {
			cont = 0;
		}
		if (out) mpt_solver_status((void *) sol, out, updateIvpData, md);
		else sol->_vptr->gen.report((void *) sol, MPT_SOLVER_ENUM(Values), updateIvpDataWrap, md);
		
	} while (neqs > 1 && cont);
	
	return err;
}

