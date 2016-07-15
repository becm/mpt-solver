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
extern int mpt_output_pde(MPT_INTERFACE(output) *out, int state, const MPT_STRUCT(value) *val, const MPT_SOLVER_STRUCT(data) *dat)
{
	const struct iovec *vec;
	const char *fmt;
	const double *grid, *y;
	size_t i, glen, ylen, ld;
	
	if (!val || !(fmt = val->fmt) || !(vec = val->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (*fmt == 'd') {
		struct {
			MPT_STRUCT(msgtype) mt;
			MPT_STRUCT(msgbind) bnd;
		} hdr;
		const double *t = val->ptr;
		int ret;
		
		hdr.mt.cmd   = MPT_ENUM(MessageValRaw);
		hdr.mt.arg   = 0;
		hdr.bnd.dim  = state & 0xff; /* special data state as dimension info */
		hdr.bnd.type = MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(t);
		
		/* push parameter data */
		if ((ret = out->_vptr->push(out, sizeof(hdr), &hdr)) >= 0) {
			if ((ret = out->_vptr->push(out, sizeof(*t), t)) < 0
			    || out->_vptr->push(out, 0, 0) < 0) {
				out->_vptr->push(out, 1, 0);
				return ret;
			}
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
		mpt_output_history(out, glen, grid, 1, y, ld);
		if (!dat || mpt_bitmap_get(dat->mask, sizeof(dat->mask), 0) <= 0) {
			mpt_output_data(out, state, 0, glen, grid, 1);
		}
	}
	for (i = 1; i <= ld; i++) {
		if (dat && mpt_bitmap_get(dat->mask, sizeof(dat->mask), i) > 0) {
			continue;
		}
		mpt_output_data(out, state, i, glen, y++, ld);
	}
	return ld;
}
