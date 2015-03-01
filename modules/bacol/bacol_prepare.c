/*!
 * check/adapt BACOL solver parameters and memory sizes
 */

#include <stdlib.h>
#include <errno.h>

#include "bacol.h"

extern int mpt_bacol_prepare(MPT_SOLVER_STRUCT(bacol) *data, int npde, int odim)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *tmp;
	int kcol, nimx, maxvec, lip, lrp, lcp;
	
	if (npde < 1 || odim < 0) {
		errno = EINVAL; return -2;
	}
	ivp->neqs = npde;
	ivp->pint = odim;
	
	/* use vector tolerances if one is set */
	odim = (npde > 1 && (data->rtol.base || data->atol.base)) ? 1 : 0;
	
	if (mpt_vecpar_cktol(&data->rtol, ivp->neqs, odim, __MPT_IVP_RTOL) < 0) {
		return -1;
	}
	if (mpt_vecpar_cktol(&data->atol, ivp->neqs, odim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	/* set helper variables */
	kcol	= data->kcol;
	nimx	= data->nintmx;
	maxvec	= npde * (nimx * kcol + MPT_BACOL_NCONTI);
	
	switch (data->backend) {
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
	    default: errno = EBADR; return -1;
	}
	
	if (!(tmp = realloc(data->x, (nimx + 1) * sizeof(*data->x)))) {
		return -1;
	}
	data->x = tmp;
	
	if (!(tmp = realloc(data->y, maxvec * sizeof(*data->y)))) {
		return -1;
	}
	data->y = tmp;
	
	if (!mpt_vecpar_alloc(&data->ipar, lip, sizeof(double)))
		return -1;
	
	if (!mpt_vecpar_alloc(&data->rpar, lrp, sizeof(int))) {
		return -1;
	}
	if (lcp && !mpt_vecpar_alloc(&data->bd.cpar, lcp, 2*sizeof(double))) {
		return -1;
	}
	((double *) data->rpar.iov_base)[0] = data->bd.tstop;
	((double *) data->rpar.iov_base)[1] = data->initstep;
	
	data->mflag.noinit = 0;
	
	return 0;
}
