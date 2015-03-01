/*!
 * get values start and number of equotations,
 */

#include <errno.h>

#include "node.h"
#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get grid data
 * 
 * Get grid address from client data.
 * 
 * \param dat  client data descriptor
 * \param neqs data needed for this number of equotations
 * 
 * \return start of grid
 */
extern double *mpt_data_grid(MPT_SOLVER_STRUCT(data) *dat, int neqs)
{
	MPT_STRUCT(buffer) *buf;
	size_t pos, need;
	int len;
	
	if (neqs < 0 || !(buf = dat->val._buf)) return 0;
	
	if (!(len = dat->nval)) {
		if (!(len = buf->used / sizeof(double))) return 0;
		dat->nval = -len;
	}
	else if (len < 0) {
		len = -len;
	}
	need  = neqs + 1;
	need *= len;
	need *= sizeof(double);
	
	if (need > (pos = buf->used)) {
		double *grid;
		if (!(grid = mpt_array_append(&dat->val, need-pos, 0)))
			return 0;
		
		buf  = dat->val._buf;
		
		if (pos % sizeof(double)) {
			grid = (double *) (buf+1);
			grid[pos/sizeof(double)] = 0.0;
		}
	}
	return (double *) (buf+1);
}
