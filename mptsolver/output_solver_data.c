/*!
 * push IVP history header
 */

#include "message.h"
#include "output.h"

#include "values.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief solver data output
 * 
 * Push solver data to output.
 * 
 * \param out    output interface descriptor
 * \param state  solver data state flags
 * \param dim    data dimension index
 * \param len    number of values
 * \param val    values base address
 * \param ld     value advance
 * 
 * \return value push result
 */
extern int mpt_output_solver_data(MPT_INTERFACE(output) *out, int state, int dim, int len, const double *val, int ld)
{
	struct {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(valsrc)  src;
	} hdr;
	int ret;
	
	if (!val || dim < 0 || dim > UINT8_MAX || len < 0) {
		return MPT_ERROR(BadArgument);
	}
	hdr.mt.cmd = MPT_MESGTYPE(ValueRaw);
	hdr.mt.arg = (int8_t) MPT_message_value(Float, *val);
	
	hdr.src.dim = dim;
	hdr.src.state = state & 0xff;
	
	if ((ret = out->_vptr->push(out, sizeof(hdr), &hdr))) {
		return ret;
	}
	if ((ret = mpt_output_values(out, len, val, ld)) < 0) {
		out->_vptr->push(out, 1, 0);
		return ret;
	}
	out->_vptr->push(out, 0, 0);
	
	return ret;
}
