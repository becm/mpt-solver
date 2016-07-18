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
	const double *par;
	double *dst;
	int np;
	
	if (!val || !dat) {
		return MPT_ERROR(BadArgument);
	}
	if (!(fmt = val->fmt)
	    || fmt[0] != MPT_value_toVector('d')) {
		return MPT_ERROR(BadType);
	}
	if (!(vec = val->ptr)) {
		return MPT_ERROR(BadValue);
	}
	if (!(par = vec->iov_base)
	    || !(np = vec->iov_len / sizeof (*par))) {
		return 0;
	}
	/* copy output so state data */
	if ((dst = mpt_data_param(dat))) {
		int len;
		if ((len = dat->npar) > np) {
			len = np;
		}
		memcpy(dst, par, len * sizeof(*par));
	}
	return np;
}
