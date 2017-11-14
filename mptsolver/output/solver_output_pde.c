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
	
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
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
extern int mpt_solver_output_pde(const MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_STRUCT(solver_data) *sd)
{
	MPT_INTERFACE(output) *dat, *grf;
	const MPT_INTERFACE(metatype) *mt;
	const MPT_STRUCT(buffer) *buf;
	const struct iovec *vec;
	const char *fmt;
	const uint8_t *pass;
	const double *grid, *y, *t;
	size_t i, glen, ylen, passlen, ld;
	
	if (!val || !(fmt = val->fmt) || !(vec = val->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	dat = 0;
	if ((mt = out->_data)) {
		mt->_vptr->conv(mt, MPT_ENUM(TypeOutput), &dat);
	}
	grf = 0;
	if ((mt = out->_graphic)) {
		mt->_vptr->conv(mt, MPT_ENUM(TypeOutput), &grf);
	}
	
	if (!dat && !out) {
		return 0;
	}
	t = 0;
	if (*fmt == 'd') {
		t = val->ptr;
		
		/* push time data */
		if (grf && (grf != dat)) {
			outputTime(grf, state, *t);
		}
		vec = (void *) (t + 1);
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
	else if (sd && (glen = sd->nval) > 0) {
		if (!ylen || !(ld = ylen / glen)) {
			return MPT_ERROR(BadValue);
		}
		grid = (const double *) (sd->val._buf + 1);
	}
	else {
		grid = 0;
		ld = 0;
	}
	if ((buf = out->_pass._buf)) {
		pass = (void *) (buf + 1);
		passlen = buf->_used / sizeof(*pass);
	} else {
		passlen = 0;
		pass = 0;
	}
	
	if (grid) {
		if (dat) {
			mpt_output_ivp_header(dat, glen, ld+1, t);
			mpt_output_solver_history(dat, grid, glen, y, ld);
		}
		if (grf
		    && (!pass || (passlen && (pass[0] & state)))) {
			mpt_output_solver_data(grf, state, 0, glen, grid, 1);
		}
	}
	else if (dat) {
		outputTime(dat, state, *t);
	}
	if (!grf) {
		return ld;
	}
	for (i = 1; i <= ld; i++) {
		if (!pass || (i < passlen && pass[i] & state)) {
			mpt_output_solver_data(grf, state, i, glen, y, ld);
		}
		++y;
	}
	return ld;
}
