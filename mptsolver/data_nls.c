/*!
 * create client for solving nonlinear systems.
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"

#include "solver.h"

extern int mpt_data_nls(MPT_SOLVER_STRUCT(data) *dat, const MPT_STRUCT(value) *val)
{
	const char *fmt;
	const struct iovec *vec;
	const double *res, *par;
	double *dst;
	int ret, lr, np;
	
	if (!val || !dat) {
		return MPT_ERROR(BadArgument);
	}
	if (!(fmt = val->fmt) || fmt[0] != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (!(vec = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	par = vec->iov_base;
	np  = vec->iov_len / sizeof (*par);
	
	ret = 0;
	res = 0;
	if (fmt[0] == MPT_value_toVector('d')) {
		res = vec[1].iov_base;
		lr  = vec[1].iov_len / sizeof(*res);
	}
	/* copy output so state data */
	if (par && (dst = mpt_data_param(dat))) {
		int len;
		if ((len = dat->npar) > np) {
			len = np;
		}
		memcpy(dst, par, len * sizeof(*par));
		ret = 1;
	}
	if (res && (dst = mpt_data_grid(dat))) {
		int len;
		if ((len = dat->nval) > lr) {
			len = len;
		}
		memcpy(dst, res, len * sizeof(*dst));
		ret = 2;
	}
	return ret;
}
