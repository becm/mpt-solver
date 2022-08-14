/*!
 * MPT solver library
 *   get solver data parameter start
 */

#include <errno.h>

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get parameter data
 * 
 * Get parameter address from client data.
 * 
 * \param md  client data descriptor
 * 
 * \return start of parameters
 */
extern double *mpt_solver_data_param(MPT_STRUCT(solver_data) *md)
{
	MPT_STRUCT(buffer) *buf;
	int len;
	
	/* parameters are initialized */
	if ((len = md->npar) <= 0 || !(buf = md->param._buf)) return 0;
	if ((len = buf->_used / sizeof(double)) < md->npar) {
		if (!(mpt_array_slice(&md->param, buf->_used, md->npar * sizeof(double)))) {
			return 0;
		}
		for (; len < md->npar; ++len) {
			((double *) (buf + 1))[len] = 0;
		}
	}
	return (double *) (buf + 1);
}
