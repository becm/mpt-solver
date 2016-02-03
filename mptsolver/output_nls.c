/*!
 * create client for solving nonlinear systems.
 */

#include <sys/uio.h>

#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "solver.h"

extern int mpt_output_nls(MPT_INTERFACE(output) *out, int state, const MPT_STRUCT(value) *val, int dest)
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
	if (par && (state & (MPT_ENUM(OutputStateInit) | MPT_ENUM(OutputStateStep) | MPT_ENUM(OutputStateFini)))) {
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
		
		mpt_output_history(out, 1, par, np, 0, 0);
	}
	/* output residuals */
	if (res && (state & (MPT_ENUM(OutputStateInit) | MPT_ENUM(OutputStateStep) | MPT_ENUM(OutputStateFini)))) {
		if (dest >= 0) {
			mpt_output_data(out, state, dest, nr, res, 1);
		}
		mpt_output_history(out, nr, 0, 0, res, 1);
	}
	return res ? 2 : 1;
}
