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
	if (out->xy.iov_base) {
		free(out->xy.iov_base);
		out->xy.iov_base = 0;
	}
	out->xy.iov_len = 0;
	
	if (out->wrk.iov_base) {
		free(out->wrk.iov_base);
		out->wrk.iov_base = 0;
	}
	out->wrk.iov_len = 0;
	
	out->nint = 0;
	out->deriv = 0;
}
