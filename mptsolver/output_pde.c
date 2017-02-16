/*!
 * create client for IVP problem types
 */

#include <string.h>

#include <sys/uio.h>

#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "solver.h"

static void outputTime(MPT_STRUCT(output) *out, int state, double t)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(msgbind) bnd;
	} hdr;
	
	hdr.mt.cmd   = MPT_ENUM(MessageValRaw);
	hdr.mt.arg   = 0; /* indicate special data */
	hdr.bnd.dim  = state & 0xff; /* repurpose dimension as special data state */
	hdr.bnd.type = MPT_message_value(Float, t);
	
	/* push parameter data */
	if (out->_vptr->push(out, sizeof(hdr), &hdr) < 0) {
		return;
	}
	out->_vptr->push(out, sizeof(t), &t);
	out->_vptr->push(out, 0, 0);
}

/*!
 * \ingroup mptSolver
 * \brief NLS data output
 * 
 * Push data state message to output.
 * 
 * \param out    nonlinear solver descriptor
 * \param state  state of PDE solver data
 * \param val    current PDE values
 * \param dat    solver data for grid and dimension mask
 * 
 * \return message push result
 */
extern int mpt_output_pde(MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_SOLVER_STRUCT(data) *dat)
{
	MPT_INTERFACE(output) *raw, *grf;
	const struct iovec *vec;
	const char *fmt;
	const double *grid, *y;
	size_t i, glen, ylen, ld;
	
	if (!val || !(fmt = val->fmt) || !(vec = val->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (!(raw = out->_data)
	    && !(grf = out->_graphic)) {
		return 0;
	}
	if (*fmt == 'd') {
		const double *t = val->ptr;
		
		/* push time data */
		if (raw) {
			outputTime(raw, state, *t);
		}
		if (grf && (grf != raw)) {
			outputTime(grf, state, *t);
		}
		vec = (void *) (t+1);
		++fmt;
	}
	if (*fmt != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	y = vec->iov_base;
	ylen = vec->iov_len / sizeof(*y);
	
	++fmt;
	
	if (*fmt == MPT_value_toVector('d')) {
		grid = y;
		
		if (!(glen = ylen)) {
			return MPT_ERROR(BadValue);
		}
		++vec;
		y = vec->iov_base;
		ylen = vec->iov_len / sizeof(*y);
		
		if (!ylen || !(ld = ylen / glen)) {
			return MPT_ERROR(BadValue);
		}
	}
	else if (dat && (glen = dat->nval) > 0) {
		if (!ylen || !(ld = ylen / glen)) {
			return MPT_ERROR(BadValue);
		}
		grid = (const double *) (dat->val._buf + 1);
	}
	else {
		grid = 0;
		ld = 0;
	}
	
	if (grid) {
		if (raw) {
			mpt_output_history(raw, glen, grid, 1, y, ld);
		}
		if (grf
		    && (!dat || mpt_bitmap_get(dat->mask, sizeof(dat->mask), 0) <= 0)) {
			mpt_output_data(grf, state, 0, glen, grid, 1);
		}
	}
	if (!grf) {
		return ld;
	}
	for (i = 1; i <= ld; i++) {
		if (dat && mpt_bitmap_get(dat->mask, sizeof(dat->mask), i) > 0) {
			continue;
		}
		mpt_output_data(grf, state, i, glen, y++, ld);
	}
	return ld;
}
