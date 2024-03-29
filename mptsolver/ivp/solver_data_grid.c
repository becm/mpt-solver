/*!
 * MPT solver library
 *   get PDE grid data
 */

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get grid data
 * 
 * Get grid address from client data.
 * 
 * \param dat  client data descriptor
 * 
 * \return start of grid
 */
extern double *mpt_solver_data_grid(MPT_STRUCT(solver_data) *dat)
{
	MPT_STRUCT(buffer) *buf;
	size_t pos, need;
	int len;
	
	if (!(buf = dat->val._buf)) {
		return 0;
	}
	if (!(len = dat->nval)) {
		if (!(len = buf->_used / sizeof(double))) {
			return 0;
		}
		dat->nval = -len;
	}
	else if (len < 0) {
		len = -len;
	}
	need = len * sizeof(double);
	
	if (need > (pos = buf->_used)) {
		double *grid;
		if (!(grid = mpt_array_append(&dat->val, need-pos, 0))) {
			return 0;
		}
		buf  = dat->val._buf;
		
		if (pos % sizeof(double)) {
			grid = (double *) (buf+1);
			grid[pos/sizeof(double)] = 0.0;
		}
	}
	return (double *) (buf + 1);
}
