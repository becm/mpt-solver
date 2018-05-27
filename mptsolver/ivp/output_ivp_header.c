/*!
 * MPT solver library
 *   push IVP history header
 */

#include "message.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief IVP data header
 * 
 * Push IVP history header data to output.
 * 
 * \param out  output interface descriptor
 * \param row  data row count
 * \param col  data column count
 * \param time PDE time for data (optional)
 * 
 * \return message push result
 */
extern int mpt_output_ivp_header(MPT_STRUCT(output) *out, int row, int col, const double *time)
{
	MPT_STRUCT(msgtype) hdr;
	int32_t val;
	uint8_t fmt;
	int len;
	
	hdr.cmd = MPT_MESGTYPE(ValueFmt);
	hdr.arg = 0;
	
	/* push header data indicator */
	if ((val = out->_vptr->push(out, sizeof(hdr), &hdr)) < 0) {
		return val;
	}
	/* history rows */
	fmt = MPT_message_value(Integer, val);
	val = row;
	out->_vptr->push(out, sizeof(fmt), &fmt);
	out->_vptr->push(out, sizeof(val), &val);
	/* history columns */
	fmt = MPT_message_value(Integer, val);
	val = col;
	out->_vptr->push(out, sizeof(fmt), &fmt);
	out->_vptr->push(out, sizeof(val), &val);
	
	len = 2 * (sizeof(fmt) + sizeof(val));
	
	/* current time index */
	if (time) {
		fmt = MPT_message_value(Float, *time);
		out->_vptr->push(out, sizeof(fmt), &fmt);
		out->_vptr->push(out, sizeof(time), time);
		len += sizeof(*time);
	}
	out->_vptr->push(out, 0, 0);

	return len;
}
