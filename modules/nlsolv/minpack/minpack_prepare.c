/*!
 * check/adapt MINPACK memory sizes and parameters
 */

#include "minpack.h"

extern int mpt_minpack_prepare(MPT_SOLVER_STRUCT(minpack) *mpack)
{
	int npar, nres;
	int lw;
	
	npar = mpack->nls.nval;
	if (!(nres = mpack->nls.nres)) {
		nres = npar;
	}
	if (npar < 1 || nres < npar) {
		return MPT_ERROR(BadArgument);
	}
	if (!mpack->solv) {
		mpack->solv = (nres != npar) ? MPT_ENUM(MinpackLmDer) : MPT_ENUM(MinpackHybrd);
	}
	/* prepare parameters and residuals */
	lw = npar + nres;
	if (!mpt_solver_module_valloc(&mpack->val, lw, sizeof(double))) {
		return MPT_ERROR(BadOperation);
	}
	/* set diagonal matrix internally */
	if ((lw = mpack->diag.iov_len/sizeof(double)) < nres) {
		mpack->mode = 1;
		if (!mpt_solver_module_valloc(&mpack->diag, nres, sizeof(double))) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* working vectors, jacobian and r */
	lw  = npar * 4;        /* qtf and wa(1..3) */
	lw += nres;            /* wa4 */
	lw += npar * nres;     /* jacobian */
	lw += npar*(npar+1)/2; /* r for hybrid or ipvt for lmderv */
	
	if (!mpt_solver_module_valloc(&mpack->work, lw, sizeof(double))) {
		return MPT_ERROR(BadOperation);
	}
	if (mpack->ml < 0) mpack->ml = npar;
	if (mpack->mu < 0) mpack->mu = npar;
	
	mpack->nfev = mpack->njev = 0;
	
	if (!mpack->maxfev) {
		mpack->maxfev = 100 * (npar + 1);
	}
	mpack->info = 0;
	
	return mpack->nls.nres ? 0 : 1;
}
