/*!
 * create client for solving nonlinear systems.
 */

#include <sys/uio.h>

#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "solver.h"

static void outputParam(MPT_STRUCT(output) *out, int state, const double *par, int np)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	
	hdr.mt.cmd   = MPT_ENUM(MessageValRaw);
	hdr.mt.arg   = 0; /* indicate special data */
	hdr.bnd.dim  = state & 0xff; /* repurpose dimension as special data state */
	hdr.bnd.type = MPT_message_value(Float, *par);
	
	/* push parameter data */
	if (out->_vptr->push(out, sizeof(hdr), &hdr) < 0) {
		return;
	}
	out->_vptr->push(out, np * sizeof(*par), par);
	out->_vptr->push(out, 0, 0);
}

/*!
 * \ingroup mptSolver
 * \brief NLS data output
 * 
 * Push data state message to output.
 * 
 * \param out    nonlinear solver descriptor
 * \param state  state of nonlinear solver data 
 * \param val    nonlinear solver parameters (and residuals)
 * \param dat    solver data for residial dimension mask
 * 
 * \return message push result
 */
extern int mpt_output_nls(MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_SOLVER_STRUCT(data) *dat)
{
	MPT_STRUCT(output) *raw, *grf;
	const char *fmt;
	const struct iovec *vec;
	const double *res, *par;
	int nr, np;
	
	if (!val) {
		return MPT_ERROR(BadArgument);
	}
	if (!(fmt = val->fmt) || fmt[0] != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (!(vec = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	if (!(raw = out->_data)
	    && !(grf = out->_graphic)) {
		return 0;
	}
	par = vec->iov_base;
	np  = vec->iov_len / sizeof (*par);
	
	res = 0;
	if (fmt[1] == MPT_value_toVector('d')) {
		res = vec[1].iov_base;
		nr  = vec[1].iov_len / sizeof(*res);
	}
	else if (fmt[1] == 'd') {
		res = (void *) (vec + 1);
		nr  = 1;
	}
	/* output parameters */
	if (par && (state & (MPT_ENUM(DataStateInit) | MPT_ENUM(DataStateStep) | MPT_ENUM(DataStateFini)))) {
		if (raw) {
			outputParam(raw, state, par, np);
		}
		if (grf && (grf != raw)) {
			outputParam(grf, state, par, np);
		}
	}
	if (!res) {
		return 1;
	}
	/* output residuals */
	if (state & MPT_ENUM(DataStateStep)) {
		if (!mpt_bitmap_get(dat->mask, sizeof(dat->mask), 0)) {
			mpt_output_data(grf, state, 0, nr, res, 1);
		}
	}
	if (state & MPT_ENUM(DataStateFini)) {
		mpt_output_history(raw, nr, res, 1, 0, 0);
	}
	/* output user data */
	if ((state & MPT_ENUM(DataStateInit))
	    && dat
	    && dat->val._buf
	    && (np = dat->val._buf->used / sizeof(double))) {
		const double *val = (double *) (dat->val._buf + 1);
		int i, nv;
		if ((nv = dat->nval) < 0) {
			np = 0;
		}
		else if (nv) {
			np /= nv;
		} else {
			nv = 1;
		}
		for (i = 0; i < np; ++i) {
			if (!mpt_bitmap_get(dat->mask, sizeof(dat->mask), i+1)) {
				mpt_output_data(grf, state, i+1, np, val+i, nv);
			}
		}
		return np + 1;
	}
	return 0;
}
