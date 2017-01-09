/*!
 * BACOL output data
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern void mpt_bacolout_init(MPT_SOLVER_STRUCT(bacolout) *out)
{
	memset(out, 0, sizeof(*out));
	
	out->update = mpt_bacol_grid_init;
}

extern void mpt_bacolout_fini(MPT_SOLVER_STRUCT(bacolout) *out)
{
	mpt_vecpar_alloc(&out->_xy, 0, 0);
	mpt_vecpar_alloc(&out->_wrk, 0, 0);
	
	out->nint = 0;
	out->deriv = 0;
}
