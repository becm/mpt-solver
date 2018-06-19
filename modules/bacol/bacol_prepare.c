/*!
 * check/adapt BACOL solver parameters and memory sizes
 */

#include <stdlib.h>

#include "bacol.h"

extern int mpt_bacol_prepare(MPT_SOLVER_STRUCT(bacol) *bac)
{
	double *tmp;
	int npde, odim, kcol, nimx, maxvec, lip, lrp, lcp;
	
	
	/* set helper variables */
	npde   = bac->ivp.neqs;
	kcol   = bac->kcol;
	nimx   = bac->nintmx;
	maxvec = npde * (nimx * kcol + MPT_BACOL_NCONTI);
	
	if (npde < 1 || bac->nint < 1 || bac->nint > nimx) {
		return MPT_ERROR(BadArgument);
	}
	/* invalidate prepared state */
	bac->mflag.noinit = -1;
	
	/* use vector tolerances if one is set */
	odim = (npde > 1 && (bac->rtol._base || bac->atol._base)) ? 1 : 0;
	
	if (mpt_solver_module_tol_check(&bac->rtol, bac->ivp.neqs, odim, __MPT_IVP_RTOL) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (mpt_solver_module_tol_check(&bac->atol, bac->ivp.neqs, odim, __MPT_IVP_ATOL) < 0) {
		return MPT_ERROR(BadOperation);
	}
	/* choose default backend */
	if (!bac->_backend && mpt_bacol_backend(bac, 0) < 0) {
		return MPT_ERROR(BadType);
	}
	
	switch (bac->_backend) {
#ifdef MPT_BACOL_RADAU
	    case 'r': case 'R':
	lip = 100 + 3 * npde * (nimx * (2 * kcol + 1) + 4);
	
	lrp = 74 + 24 * npde * npde * nimx * kcol;
	lrp += 8 * npde * npde * nimx * kcol * kcol;
	lrp += 29 * npde * nimx * kcol + 61 * npde + 14 * kcol;
	lrp += 35 * nimx * kcol;
	lrp += 35 * nimx + 21 * nimx * npde + 8 * nimx * kcol * kcol;
	lrp += 37 * npde * npde + 12 * npde * npde * nimx;
	
	lcp = npde * (4 + (2 * kcol + 1) * nimx);
	lcp += npde * npde * (8 + nimx * (2 * kcol * (kcol + 3) + 3));
	    break;
#endif
	
#ifdef MPT_BACOL_DASSL
	    case 'd': case 'D':
	lip = 115 + npde * ((2 * kcol + 1) * nimx + 4);
	
	lrp = 134 + nimx * (35 + 35 * kcol + 31 * npde + 38 * npde * kcol + 8 * kcol * kcol);
	lrp += 14 * kcol + 79 * npde;
	lrp += npde * npde * (21 + 4 * nimx * kcol * kcol + 12 * nimx * kcol + 6 * nimx);
	
	lcp = 0;
	    break;
#endif
	    default:
	return MPT_ERROR(BadArgument);
	}
	
	if (!(tmp = realloc(bac->xy, (nimx + 1 + maxvec) * sizeof(*bac->xy)))) {
		return MPT_ERROR(BadOperation);
	}
	bac->xy = tmp;
	
	mpt_bacol_grid_init(bac->ivp.pint, bac->ivp.grid, bac->nint, tmp);
	
	if (!mpt_solver_module_valloc(&bac->ipar, lip, sizeof(int))) {
		return MPT_ERROR(BadOperation);
	}
	if (!mpt_solver_module_valloc(&bac->rpar, lrp, sizeof(double))) {
		return MPT_ERROR(BadOperation);
	}
	if (lcp && !mpt_solver_module_valloc(&bac->bd.cpar, lcp, 2*sizeof(double))) {
		return MPT_ERROR(BadOperation);
	}
	((double *) bac->rpar.iov_base)[0] = bac->bd.tstop;
	((double *) bac->rpar.iov_base)[1] = bac->initstep;
	
	bac->mflag.noinit = 0;
	
	return 0;
}
