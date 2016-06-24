/*!
 * create client for solving nonlinear systems.
 */

#include <sys/uio.h>

#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "solver.h"

extern int mpt_output_nls(MPT_INTERFACE(output) *out, int state, const MPT_STRUCT(value) *val, const MPT_SOLVER_STRUCT(data) *dat)
{
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
	par = vec->iov_base;
	np  = vec->iov_len / sizeof (*par);
	
	res = 0;
	if (fmt[0] == MPT_value_toVector('d')) {
		res = vec[1].iov_base;
		nr  = vec[1].iov_len / sizeof(*res);
	}
	else if (fmt[0] == 'd') {
		res = (void *) (vec + 1);
		nr  = 1;
	}
	/* output parameters */
	if (par && (state & (MPT_ENUM(DataStateInit) | MPT_ENUM(DataStateStep) | MPT_ENUM(DataStateFini)))) {
		struct {
			MPT_STRUCT(msgtype) mt;
			MPT_STRUCT(msgbind) bnd;
		} hdr;
		
		hdr.mt.cmd   = MPT_ENUM(MessageValRaw);
		hdr.mt.arg   = 0; /* indicate special data */
		hdr.bnd.dim  = state & 0xff; /* repurpose dimension as special data state */
		hdr.bnd.type = MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(*par);
		
		/* push parameter data */
		out->_vptr->push(out, sizeof(hdr), &hdr);
		out->_vptr->push(out, np * sizeof(*par), par);
		out->_vptr->push(out, 0, 0);
	}
	if (!res) {
		return 1;
	}
	/* output residuals */
	if (state & MPT_ENUM(DataStateStep)) {
		if (!dat || !mpt_bitmap_get(dat->mask, sizeof(dat->mask), 0)) {
			mpt_output_data(out, state, 0, nr, res, 1);
		}
	}
	if (dat && dat->val._buf &&
	    (np = dat->val._buf->used / sizeof(double))) {
		if (dat->nval && nr > dat->nval) {
			nr = dat->nval;
		}
		np /= nr;
		par = np ? ((double *) (dat->val._buf + 1)) : 0;
	}
	if (state & MPT_ENUM(DataStateInit)) {
		int i;
		for (i = 0; i < np; ++i) {
			if (!dat || !mpt_bitmap_get(dat->mask, sizeof(dat->mask), i+1)) {
				mpt_output_data(out, state, i+1, nr, par+i, 1);
			}
		}
	}
	if (state & MPT_ENUM(DataStateFini)) {
		mpt_output_history(out, nr, res, 1, par, np);
	}
	return np ? 3 : 2;
}
