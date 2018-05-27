/*!
 * MPT solver library
 *   output of floating point data.
 */

#include <string.h>
#include <sys/uio.h>

#include "message.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \~english
 * \brief send history data
 * 
 * Send binary data to output descriptor.
 * Pass zero pointer for <b>val</b> for contigous reference and value data
 * 
 * \param out  output descriptor
 * \param ref  address of reference data
 * \param len  number of history blocks
 * \param val  address of value data
 * \param vlen number of value elements per block
 * 
 * \return number of used messages for data
 */
extern int mpt_output_solver_history(MPT_INTERFACE(output) *out, const double *ref, int len, const double *val, int vlen)
{
	MPT_STRUCT(msgtype) hdr;
	int i, plen;
	uint8_t fmt;
	
	if (len <= 0) {
		return MPT_ERROR(BadArgument);
	}
	plen = ref ? 1 : 0;
	
	/* values in row order */
	if (vlen < 0) {
		plen -= vlen;
	} else {
		plen += vlen;
	}
	/* header setup for data output */
	hdr.cmd = MPT_MESGTYPE(ValueFmt);
	hdr.arg = plen;
	fmt = MPT_message_value(Float, *ref);
	
	/* push header and format information */
	out->_vptr->push(out, sizeof(hdr), &hdr);
	for (i = 0; i < plen; i++) {
		out->_vptr->push(out, sizeof(fmt), &fmt);
	}
	for (i = 0; i < len; i++) {
		/* push reference data */
		if (ref) {
			out->_vptr->push(out, sizeof(*ref), ref++);
		}
		if (!val) {
			continue;
		}
		/* no value advance */
		if (!vlen) {
			out->_vptr->push(out, vlen * sizeof(*val), val);
			continue;
		}
		/* push values in row order */
		if (vlen < 0) {
			int j, max;
			for (j = 0, max = -vlen; j < max; ++j) {
				out->_vptr->push(out, sizeof(*val), val + i * len);
			}
			++val;
		}
		/* push values in column order */
		else {
			out->_vptr->push(out, vlen * sizeof(*val), val);
			val += vlen;
		}
	}
	out->_vptr->push(out, 0, 0);
	
	return 0;
}
