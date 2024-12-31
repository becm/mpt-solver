/*!
 * MPT solver library
 *   output nonlinear systems data
 */

#include <sys/uio.h>

#include "message.h"

#include "values.h"
#include "output.h"

#include "solver.h"

static void outputSize(MPT_STRUCT(output) *out, int nres)
{
	MPT_STRUCT(msgtype) hdr;
	uint32_t val;
	uint8_t fmt;
	int len;
	
	hdr.cmd = MPT_MESGTYPE(ValueFmt);
	hdr.arg = 0;
	
	/* push header data indicator */
	if ((len = out->_vptr->push(out, sizeof(hdr), &hdr)) < 0) {
		return;
	}
	/* append residual size */
	fmt = MPT_message_value(Unsigned, val);
	val = nres;
	out->_vptr->push(out, sizeof(fmt), &fmt);
	out->_vptr->push(out, sizeof(val), &val);
	/* terminate message */
	out->_vptr->push(out, 0, 0);
}

static void outputParam(MPT_STRUCT(output) *out, int state, const double *par, int np)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(valsrc)  src;
	} hdr;
	int i;
	
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, *par);
	
	/* indicate special data */
	hdr.src.dim = state;
	hdr.src.state = 0;
	
	/* push header */
	if (out->_vptr->push(out, sizeof(hdr), &hdr) < 0) {
		return;
	}
	/* append values */
	for (i = 0; i < np; ++i) {
		if (out->_vptr->push(out, sizeof(*par), par++) < 0) {
			out->_vptr->push(out, 1, 0);
		}
	}
	/* terminate message */
	out->_vptr->push(out, 0, 0);
}
static void outputValues(MPT_STRUCT(output) *out, int state, int dim, int len, const double *val, int ld)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(valsrc)  src;
	} hdr;
	
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, *val);
	
	hdr.src.dim = dim & 0xff;
	hdr.src.state = state & 0xff;
	
	out->_vptr->push(out, sizeof(hdr), &hdr);
	mpt_output_values(out, len, val, ld);
	out->_vptr->push(out, 0, 0);
}

/*!
 * \ingroup mptSolver
 * \brief NLS data output
 * 
 * Push data to solver output and graphic.
 * 
 * \param out    nonlinear solver descriptor
 * \param state  state of nonlinear solver data
 * \param val    nonlinear solver parameters (and residuals)
 * 
 * \return message push result
 */
extern int mpt_solver_output_nls(const MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(value) *val)
{
	const struct iovec *vec;
	const double *res, *par;
	int nr, np;
	
	if (!val || !(vec = val->_addr)) {
		return MPT_ERROR(BadArgument);
	}
	par = 0;
	res = 0;
	
	if (val->_type == MPT_type_toVector('d')) {
		par = vec->iov_base;
		np  = vec->iov_len / sizeof (*par);
	}
	else if (val->_type == MPT_ENUM(TypeObjectPtr)) {
		MPT_STRUCT(property) pr;
		const MPT_INTERFACE(object) *obj = *((void * const *) val->_addr);
		
		pr.name = "parameters";
		if (obj->_vptr->property(obj, &pr) >= 0
		 && pr.val._type == MPT_type_toVector('d')
		 && pr.val._addr) {
			vec = pr.val._addr;
			par = vec->iov_base;
			np  = vec->iov_len / sizeof (*par);
		}
		pr.name = "residuals";
		if (obj->_vptr->property(obj, &pr) >= 0
		 && pr.val._type == MPT_type_toVector('d')
		 && pr.val._addr) {
			vec = pr.val._addr;
			res = vec->iov_base;
			nr  = vec->iov_len / sizeof (*par);
		}
	}
	
	if (!par) {
		return MPT_ERROR(BadValue);
	}
	if (!out->_data && !out->_graphic) {
		return 0;
	}
	/* output parameters */
	if (par
	 && (state & (MPT_DATASTATE(Init) | MPT_DATASTATE(Step) | MPT_DATASTATE(Fini)))) {
		if (out->_data) {
			outputParam(out->_data, state, par, np);
		}
		if (out->_graphic
		 && out->_graphic != out->_data) {
			/* pass state via dimension */
			outputParam(out->_graphic, state, par, np);
		}
	}
	if (!res) {
		return 1;
	}
	/* output residuals */
	if (out->_graphic) {
		const MPT_STRUCT(buffer) *buf;
		if ((buf = out->_pass._buf)) {
			const uint8_t *pass = (void *) (buf + 1);
			size_t passlen = buf->_used / sizeof(uint8_t);
			if (!pass || !passlen || (pass[0] & state)) {
				outputValues(out->_graphic, state, 0, nr, res, 1);
			}
		} else if (state & (MPT_DATASTATE(Init) & MPT_DATASTATE(Step) & MPT_DATASTATE(Fini))) {
			outputValues(out->_graphic, state, 0, nr, res, 1);
		}
	}
	if (out->_data
	 && state & MPT_DATASTATE(Fini)) {
		outputSize(out->_data, nr);
		mpt_output_solver_history(out->_data, res, nr, 0, 0);
	}
	return 0;
}

/*!
 * \ingroup mptSolver
 * \brief plot NLS input data
 * 
 * Push reference data to graphic output.
 * 
 * \param out    nonlinear solver descriptor
 * \param state  state for reference data
 * \param sd     solver data storage
 * 
 * \return message push result
 */
extern int mpt_solver_output_nlsdata(const MPT_STRUCT(solver_output) *out, int state, const MPT_STRUCT(solver_data) *sd)
{
	const MPT_STRUCT(buffer) *buf;
	const uint8_t *pass;
	const double *val;
	size_t passlen, all, i;
	int add, nv;
	
	if (!out->_graphic || !sd) {
		return 0;
	}
	
	if ((buf = out->_pass._buf)) {
		pass = (void *) (buf + 1);
		passlen = buf->_used / sizeof(uint8_t);
	} else {
		pass = 0;
		passlen = 0;
	}
	add = 0;
	if (!(buf = sd->val._buf)
	 || !(val = (double *) (buf + 1))
	 || !(all = buf->_used / sizeof(double))) {
		 return 0;
	 }
	
	/* segment and/or align dimensions */
	if ((nv = sd->nval) < 0) {
		all = 0;
	}
	else if (nv) {
		all /= nv;
	} else {
		nv = 1;
	}
	
	/* output reference data at dimension 1..n */
	for (i = 0; i < all; ++i) {
		size_t pos = i + 1;
		if (!pass || (pos < passlen && pass[pos] & state)) {
			outputValues(out->_graphic, state, pos, all, val + i, nv);
			++add;
		}
	}
	return add;
}
