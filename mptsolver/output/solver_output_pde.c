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
	
	hdr.mt.cmd = MPT_ENUM(MessageValRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, t);
	
	/* indicate special data */
	hdr.bnd.dim = state;
	hdr.bnd.state = 0;
	
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
 * \param out    solver output data
 * \param state  state of PDE solver data
 * \param val    current PDE values
 * \param dat    solver data for grid
 * 
 * \return message push result
 */
extern int mpt_solver_output_pde(MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_STRUCT(solver_data) *dat)
{
	MPT_INTERFACE(output) *raw, *grf;
	const MPT_STRUCT(buffer) *buf;
	const struct iovec *vec;
	const char *fmt;
	const uint8_t *pass;
	const double *grid, *y, *t;
	size_t i, glen, ylen, passlen, ld;
	
	if (!val || !(fmt = val->fmt) || !(vec = val->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	raw = out->_data;
	grf = out->_graphic;
	
	if (!raw && !out) {
		return 0;
	}
	t = 0;
	if (*fmt == 'd') {
		t = val->ptr;
		
		/* push time data */
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
	if ((buf = out->_pass._buf)) {
		pass = (void *) (buf + 1);
		passlen = buf->used;
	} else {
		passlen = 0;
		pass = 0;
	}
	
	if (grid) {
		if (raw) {
			mpt_output_ivp_header(raw, glen, ld+1, t);
			mpt_output_history(raw, grid, glen, y, ld);
		}
		if (grf
		    && (!passlen || mpt_bitmap_get(pass, passlen, 0) > 0)) {
			mpt_output_solver_data(grf, state, 0, glen, grid, 1);
		}
	}
	else if (raw) {
		outputTime(raw, state, *t);
	}
	if (!grf) {
		return ld;
	}
	for (i = 1; i <= ld; i++) {
		if (!passlen || mpt_bitmap_get(pass, passlen, i) > 0) {
			mpt_output_solver_data(grf, state, i, glen, y, ld);
		}
		++y;
	}
	return ld;
}