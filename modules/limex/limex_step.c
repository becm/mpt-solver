/*!
 * C wrapper to LIMEX fortran routine
 */

#include "limex.h"

extern int mpt_limex_step(MPT_SOLVER_STRUCT(limex) *lx, double tend)
{
	double *atol, *rtol;
	struct {
		int neqs, pint;
		const void *ufcn;
	} neq;
	
	if (lx->iopt[15] < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (!lx->fcn) {
		return MPT_ERROR(BadArgument);
	}
	neq.neqs = lx->ivp.neqs * (lx->ivp.pint + 1);
	neq.pint = lx->ivp.pint;
	neq.ufcn = lx->ufcn;
	
	/* set tolerance flags and adresses */
	if (neq.neqs > 1 && (rtol = lx->rtol.base) && (atol = lx->atol.base)) {
		lx->iopt[10] = 1;
	} else {
		lx->iopt[10] = 0;
		rtol = &lx->rtol.d.val;
		atol = &lx->atol.d.val;
	}
	if (!lx->jac) {
		lx->iopt[6] = 0;
	}
	/* fortran routine call */
	limex_(&neq.neqs, lx->fcn, lx->jac, &lx->t, &tend, lx->y, lx->ys,
	       rtol, atol, &lx->h, lx->iopt, lx->ropt, lx->ipos, lx->ifail.raw);
	
	if (lx->ifail.st.code < 0) {
		return lx->ifail.st.code;
	}
	return 0;
}
