/*!
 * create client for solving nonlinear systems.
 */

#include <sys/uio.h>

#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "solver.h"

static void outputParam(MPT_STRUCT(output) *out, const double *par, int np)
{
	MPT_STRUCT(msgtype) mt;
	
	mt.cmd = MPT_ENUM(MessageValRaw);
	mt.arg = 0;
	
	/* push parameter data */
	if (out->_vptr->push(out, sizeof(mt), &mt) < 0) {
		return;
	}
	while (np-- > 0) {
		uint8_t fmt = (int8_t) MPT_message_value(Float, *par);
		out->_vptr->push(out, sizeof(fmt), &fmt);
		out->_vptr->push(out, sizeof(*par), par++);
	}
	out->_vptr->push(out, 0, 0);
}
static void outputValues(MPT_STRUCT(output) *out, int state, int dim, int len, const double *val, int ld)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	
	hdr.mt.cmd = MPT_ENUM(MessageValRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, *val);
	
	hdr.bnd.dim = dim & 0xff;
	hdr.bnd.state = state & 0xff;
	
	out->_vptr->push(out, sizeof(hdr), &hdr);
	mpt_output_values(out, len, val, ld);
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
extern int mpt_solver_output_nls(MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_STRUCT(solver_data) *dat)
{
	MPT_INTERFACE(output) *raw, *grf;
	const MPT_STRUCT(buffer) *buf;
	const uint8_t *pass;
	const char *fmt;
	const struct iovec *vec;
	const double *res, *par;
	size_t passlen;
	int nr, np;
	
	if (!val) {
		return MPT_ERROR(BadArgument);
	}
	if (!(fmt = val->fmt)
	    || fmt[0] != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (!(vec = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	raw = out->_data;
	grf = out->_graphic;
	
	if (!raw && !out) {
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
	if (par &&
	    (state & (MPT_ENUM(DataStateInit) | MPT_ENUM(DataStateStep) | MPT_ENUM(DataStateFini)))) {
		if (raw) {
			outputParam(raw, par, np);
		}
		if (grf) {
			/* pass state via dimension */
			outputValues(grf, 0, state, np, par, 1);
		}
	}
	if (!res) {
		return 1;
	}
	if ((buf = out->_pass._buf)) {
		pass = (void *) (buf + 1);
		passlen = buf->used;
	} else {
		pass = 0;
		passlen = 0;
	}
	/* output residuals */
	if (grf
	    && state & MPT_ENUM(DataStateStep)) {
		if (!passlen || mpt_bitmap_get(pass, passlen, 0) > 0) {
			outputValues(grf, 0, state, nr, res, 1);
		}
	}
	if (raw
	    && state & MPT_ENUM(DataStateFini)) {
		mpt_output_history(raw, res, nr, 0, 0);
	}
	/* output user data */
	if (grf
	    && (state & MPT_ENUM(DataStateInit))
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
			if (!passlen || mpt_bitmap_get(pass, passlen, i+1) > 0) {
				outputValues(grf, state, i+1, np, val+i, nv);
			}
		}
		return np + 1;
	}
	return 0;
}