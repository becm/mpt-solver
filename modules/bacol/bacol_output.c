/*!
 * BACOL output data
 */

#include <stdlib.h>
#include <string.h>

#include "types.h"

#include "bacol.h"

extern int mpt_bacol_output_grid(const MPT_SOLVER_STRUCT(bacol_out) *bo, MPT_STRUCT(value) *val)
{
	struct iovec *vec;
	if (!bo->nint) {
		return MPT_ERROR(BadArgument);
	}
	if (val->_bufsize < sizeof(*vec)) {
		return MPT_ERROR(MissingBuffer);
	}
	val->domain = 0;
	val->type = MPT_type_toVector('d');
	val->ptr = vec = (void *) val->_buf;
	
	vec->iov_base = bo->_val.iov_base;
	vec->iov_len  = (bo->nint + 1) * sizeof(double);
	
	return bo->neqs;
}

extern int mpt_bacol_output_values(const MPT_SOLVER_STRUCT(bacol_out) *bo, MPT_STRUCT(value) *val)
{
	struct iovec *vec;
	double *u;
	long len;
	if (!bo->nint || !bo->neqs || !(u = bo->_val.iov_base)) {
		return MPT_ERROR(BadArgument);
	}
	if (val->_bufsize < sizeof(*vec)) {
		return MPT_ERROR(MissingBuffer);
	}
	val->domain = 0;
	val->type = MPT_type_toVector('d');
	val->ptr = vec = (void *) val->_buf;

	len = (bo->nint + 1) * bo->neqs;
	vec->iov_base = u + bo->nint + 1;
	vec->iov_len = len * sizeof(*u);
	
	return bo->neqs;
}

extern void mpt_bacol_output_init(MPT_SOLVER_STRUCT(bacol_out) *out)
{
	memset(out, 0, sizeof(*out));
	
	out->update = mpt_bacol_grid_init;
}

extern void mpt_bacol_output_fini(MPT_SOLVER_STRUCT(bacol_out) *out)
{
	mpt_solver_module_valloc(&out->_val, 0, 0);
	mpt_solver_module_valloc(&out->_wrk, 0, 0);
	
	out->nint = 0;
	out->neqs = 0;
	out->deriv = 0;
}
