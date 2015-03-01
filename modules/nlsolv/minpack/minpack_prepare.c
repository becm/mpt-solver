/*!
 * check/adapt MINPACK memory sizes and parameters
 */

#include <stdlib.h>
#include <errno.h>

#include "minpack.h"

extern int mpt_minpack_prepare(MPT_SOLVER_STRUCT(minpack) *mpack, int n, int m)
{
	MPT_SOLVER_STRUCT(nlspar) *nl = &mpack->nls;
	int lw;
	
	if (!m) m = n;
	
	if (n < 1 || m < n) {
		errno = EINVAL;
		return -2;
	}
	
	if (!mpack->solv) {
		mpack->solv = (m != n) ? MPT_ENUM(MinpackLmDer) : MPT_ENUM(MinpackHybrd);
	}
	/* set diagonal matrix internally */
	if ((lw = mpack->diag.iov_len/sizeof(double)) < n) {
		mpack->mode = 1;
		if (!mpt_vecpar_alloc(&mpack->diag, n, sizeof(double))) {
			return -1;
		}
	}
	/* working vectors, jacobian and r */
	lw  = n * 4;	/* qtf and wa(1..3) */
	lw += m;	/* wa4 */
	lw += n * m;	/* jacobian */
	lw += n*(n+1)/2;/* r for hybrid or ipvt for lmderv */
	
	if (!mpt_vecpar_alloc(&mpack->work, lw, sizeof(double))) {
		return -1;
	}
	nl->nval = n;
	nl->nres = m ? m : n;
	
	if (mpack->ml < 0) mpack->ml = n;
	if (mpack->mu < 0) mpack->mu = n;
	
	mpack->nfev = mpack->njev = 0;
	
	if (!mpack->maxfev) {
		mpack->maxfev = 100 * (n + 1);
	}
	mpack->info = 0;
	
	return nl->nres ? 0 : 1;
}
