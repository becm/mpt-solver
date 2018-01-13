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
extern int mpt_solver_output_ode(const MPT_STRUCT(solver_output) *so, int state, const MPT_STRUCT(solver_data) *sd)
{
	const MPT_STRUCT(buffer) *buf;
	MPT_INTERFACE(output) *out;
	const double *val;
	int ld, len;
	
	if (!sd
	    || !(buf = sd->val._buf)) {
		return 0;
	}
	if (!(len = buf->_used / sizeof(*val))) {
		return 0;
	}
	if (!(ld = sd->nval)
	    || !(len /= ld)) {
		return MPT_ERROR(MissingData);
	}
	val = (const void *) (buf + 1);
	
	if ((out = so->_data)) {
		mpt_output_ivp_header(out, len, ld, 0);
		mpt_output_solver_history(out, 0, len, val, ld);
	}
	if ((out = so->_graphic)) {
		const uint8_t *pass;
		size_t passlen;
		int i, nout = 0;
		
		if ((buf = so->_pass._buf)) {
			pass = (void *) (buf + 1);
			passlen = buf->_used / sizeof(*pass);
		} else {
			pass = 0;
			passlen = 0;
		}
		for (i = 0; i < ld; i++) {
			if (pass && ((size_t) i >= passlen || !(pass[i] & state))) {
				continue;
			}
			mpt_output_solver_data(out, state, i, len, val + i, ld);
			++nout;
		}
		return nout;
	}
	return 0;
}
