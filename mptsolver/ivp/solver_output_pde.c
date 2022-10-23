/*!
 * MPT solver library
 *   output PDE data
 */

#include <string.h>

#include <sys/uio.h>

#include "message.h"

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
	struct iovec grid, y;
	const uint8_t *pass;
	const double *t;
	double _t = 0.0;
	long i, len, passlen, ld;
	int type;
	
	if (!val || !MPT_value_isBaseType(val) || !val->_addr) {
		return MPT_ERROR(BadArgument);
	}
	if (!out->_data && !out->_graphic) {
		return 0;
	}
	type = val->_type;
	t = 0;
	len = 0;
	y.iov_base = 0;
	grid.iov_base = 0;
	if (type == 'd') {
		t = val->_addr;
		
		/* push time data */
		if (out->_graphic
		 && (out->_graphic != out->_data)) {
			outputTime(out->_graphic, state, *t);
		}
	}
	else if (type == MPT_type_toVector('d')) {
		const struct iovec *vec = val->_addr;
		y = *vec;
		len = y.iov_len / sizeof(double);
	}
	else if (type == MPT_ENUM(TypeObjectPtr)) {
		const MPT_STRUCT(object) *obj = *((void * const *) val->_addr);
		MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
		
		pr.name = "t";
		if (obj->_vptr->property(obj, &pr) > 0
		 && pr.val._addr
		 && MPT_value_isBaseType(&pr.val)
		 && pr.val._type == 'd') {
			t = memcpy(&_t, pr.val._addr, sizeof(*t));
		}
		pr.name = "y";
		if (obj->_vptr->property(obj, &pr) > 0
		 && pr.val._addr
		 && MPT_value_isBaseType(&pr.val)
		 && pr.val._type == MPT_type_toVector('d')) {
			memcpy(&y, pr.val._addr, sizeof(y));
			len = y.iov_len / sizeof(double);
		}
		pr.name = "grid";
		if (obj->_vptr->property(obj, &pr) > 0
		 && pr.val._addr
		 && MPT_value_isBaseType(&pr.val)
		 && pr.val._type == MPT_type_toVector('d')) {
			memcpy(&grid, pr.val._addr, sizeof(grid));
		}
	}
	
	if (grid.iov_base) {
		if (!(len = grid.iov_len / sizeof(double))) {
			return MPT_ERROR(BadValue);
		}
		if (!(ld = y.iov_len / grid.iov_len)) {
			return MPT_ERROR(BadValue);
		}
	}
	else if (sd && (len = sd->nval) > 0) {
		if (!(ld = y.iov_len / len)) {
			return MPT_ERROR(BadValue);
		}
		grid.iov_base = (double *) (sd->val._buf + 1);
	}
	else {
		len = 0;
		ld = 0;
	}
	if ((buf = out->_pass._buf)) {
		pass = (void *) (buf + 1);
		passlen = buf->_used / sizeof(*pass);
	} else {
		passlen = 0;
		pass = 0;
	}
	
	if (grid.iov_base) {
		if (out->_data) {
			mpt_output_ivp_header(out->_data, len, ld + 1, t);
			mpt_output_solver_history(out->_data, grid.iov_base, len, y.iov_base, ld);
		}
		if (out->_graphic
		    && (!pass || (passlen && (pass[0] & state)))) {
			mpt_output_solver_data(out->_graphic, state, 0, len, grid.iov_base, 1);
		}
	}
	else if (out->_data) {
		outputTime(out->_data, state, *t);
	}
	if (!out->_graphic) {
		return ld;
	}
	type = 0;
	for (i = 0; i < ld; i++) {
		const double *curr = y.iov_base;
		int pos = i + 1;
		if (!pass || (pos < passlen && pass[pos] & state)) {
			mpt_output_solver_data(out->_graphic, state, pos, len, curr + i, ld);
			++type;
		}
	}
	return type;
}
