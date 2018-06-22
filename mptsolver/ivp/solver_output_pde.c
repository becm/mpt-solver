/*!
 * MPT solver library
 *   output PDE data
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
		MPT_STRUCT(valsrc)  src;
	} hdr;
	
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, t);
	
	/* indicate special data */
	hdr.src.dim = state;
	hdr.src.state = 0;
	
	/* push parameter data */
	if (out->_vptr->push(out, sizeof(hdr), &hdr) < 0) {
		return;
	}
	out->_vptr->push(out, sizeof(t), &t);
	out->_vptr->push(out, 0, 0);
}
/*!
 * \ingroup mptSolver
 * \brief PDE data output
 * 
 * History and graphic output for PDE data.
 * 
 * \param out    solver output data
 * \param state  state of PDE solver data
 * \param val    current PDE values
 * \param sd     solver data (fallback grid)
 * 
 * \return message push result
 */
extern int mpt_solver_output_pde(const MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val, const MPT_STRUCT(solver_data) *sd)
{
	const MPT_STRUCT(buffer) *buf;
	const struct iovec *vec;
	const uint8_t *fmt;
	const uint8_t *pass;
	const double *grid, *y, *t;
	size_t i, glen, ylen, passlen, ld;
	
	if (!val || !(fmt = val->fmt) || !(vec = val->ptr)) {
		return MPT_ERROR(BadArgument);
	}
	if (!out->_data && !out->_graphic) {
		return 0;
	}
	t = 0;
	if (*fmt == 'd') {
		t = val->ptr;
		
		/* push time data */
		if (out->_graphic
		    && (out->_graphic != out->_data)) {
			outputTime(out->_graphic, state, *t);
		}
		vec = (void *) (t + 1);
		++fmt;
	}
	if (*fmt != MPT_type_vector('d')) {
		return MPT_ERROR(BadType);
	}
	y = vec->iov_base;
	ylen = vec->iov_len / sizeof(*y);
	
	++fmt;
	
	if (*fmt == MPT_type_vector('d')) {
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
		if (!(ld = ylen / glen)) {
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
		if (out->_data) {
			mpt_output_ivp_header(out->_data, glen, ld + 1, t);
			mpt_output_solver_history(out->_data, grid, glen, y, ld);
		}
		if (out->_graphic
		    && (!pass || (passlen && (pass[0] & state)))) {
			mpt_output_solver_data(out->_graphic, state, 0, glen, grid, 1);
		}
	}
	else if (out->_data) {
		outputTime(out->_data, state, *t);
	}
	if (!out->_graphic) {
		return ld;
	}
	ylen = 0;
	for (i = 1; i <= ld; i++) {
		if (!pass || (i < passlen && pass[i] & state)) {
			mpt_output_solver_data(out->_graphic, state, i, glen, y, ld);
			++ylen;
		}
		++y;
	}
	return ylen;
}
