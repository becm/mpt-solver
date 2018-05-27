/*!
 * MPT solver library
 *   save nonlinear system parameters
 */

#include <string.h>

#include <sys/uio.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief save nonlinear system parameters
 * 
 * Save content of first value element to solver data parameters.
 * 
 * \param dat  solver data
 * \param val  current nonlinear solver values
 * 
 * \return number of changed elements
 */
extern int mpt_solver_data_nls(MPT_STRUCT(solver_data) *dat, const MPT_STRUCT(value) *val)
{
	const uint8_t *fmt;
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
	if ((dst = mpt_solver_data_param(dat))) {
		int len;
		if ((len = dat->npar) > np) {
			len = np;
		}
		memcpy(dst, par, len * sizeof(*par));
	}
	return np;
}
