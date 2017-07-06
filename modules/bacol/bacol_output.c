/*!
 * BACOL output data
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_output_report(const MPT_SOLVER_STRUCT(bacol_out) *bo, double t, int (*out)(void *, MPT_STRUCT(value)), void *usr)
{
	MPT_STRUCT(value) val;
	struct iovec *vec;
	uint8_t buf[sizeof(t) + 2 * sizeof(*vec)];
	double *u;
	long len;
	char fmt[] = { 'd', MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
	
	if (!bo->nint) {
		return MPT_ERROR(BadArgument);
	}
	val.fmt = fmt;
	val.ptr = memcpy(buf, &t, sizeof(t));
	
	vec = (void *) (buf + sizeof(t));
	u   = bo->_val.iov_base;
	
	len = bo->nint + 1;
	vec->iov_base = u;
	vec->iov_len = len * sizeof(*u);
	++vec;
	len *= bo->neqs;
	vec->iov_base = u + bo->nint + 1;
	vec->iov_len = len * sizeof(*u);
	
	return out(usr, val);
}

extern void mpt_bacol_output_init(MPT_SOLVER_STRUCT(bacol_out) *out)
{
	memset(out, 0, sizeof(*out));
	
	out->update = mpt_bacol_grid_init;
}

extern void mpt_bacol_output_fini(MPT_SOLVER_STRUCT(bacol_out) *out)
{
	mpt_solver_valloc(&out->_val, 0, 0);
	mpt_solver_valloc(&out->_wrk, 0, 0);
	
	out->nint = 0;
	out->deriv = 0;
}
