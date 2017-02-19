/*!
 * push IVP history header
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
extern int mpt_solver_output_ode(MPT_STRUCT(solver_output) *so, int state, const MPT_STRUCT(solver_data) *dat)
{
	MPT_STRUCT(buffer) *buf;
	MPT_STRUCT(output) *out;
	const double *val;
	int ld, len;
	
	if (!dat
	    || !(buf = dat->val._buf)) {
		return 0;
	}
	if (!(len = buf->used / sizeof(*val))) {
		return 0;
	}
	if (!(ld = dat->nval)
	    || !(len /= ld)) {
		return MPT_ERROR(MissingData);
	}
	val = (void *) (buf + 1);
	
	if ((out = so->_data)) {
		mpt_output_ivp_header(out, len, ld, 0);
		mpt_output_history(out, 0, len, val, ld);
	}
	if ((out = so->_graphic)) {
		const uint8_t *pass;
		size_t passlen;
		int i, dat = 0;
		
		if ((buf = so->_pass._buf)) {
			pass = (void *) (buf + 1);
			passlen = buf->used;
		} else {
			pass = 0;
			passlen = 0;
		}
		for (i = 0; i < ld; i++) {
			if (len && mpt_bitmap_get(pass, passlen, i) <= 0) {
				continue;
			}
			mpt_output_solver_data(out, state, i, len, val+i, ld);
			++dat;
		}
		return dat;
	}
	return 0;
}
