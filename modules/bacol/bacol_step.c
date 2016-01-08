/*!
 * C wrapper to bacol fortran routines
 */

#include <errno.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_step(MPT_SOLVER_STRUCT(bacol) *bac, double tend)
{
	double *rtol, *atol;
	int idid, kcol, lip, lrp;
#ifdef MPT_BACOL_RADAU
	int lcp;
#endif
	if ((idid = bac->mflag.noinit) < 0) {
		return MPT_ERROR(BadArgument);
	}
	if (idid) {
		bac->mflag.noinit = 1;
		idid = 1;
	}
	if (bac->rtol.base && bac->atol.base
	    && bac->ivp.neqs > 1) {
		bac->mflag.tvec = 1;
		rtol = bac->rtol.base;
		atol = bac->atol.base;
	}
	else {
		bac->mflag.tvec = 0;
		rtol = &bac->rtol.d.val;
		atol = &bac->atol.d.val;
	}
	kcol = bac->kcol;
	
	lrp = bac->rpar.iov_len / sizeof(double);
	lip = bac->ipar.iov_len / sizeof(int);
	
	/* fortran routine call */
	switch (bac->backend) {
#ifdef MPT_BACOL_RADAU
	    case 'r': case 'R':
	lcp = bac->bd.cpar.iov_len / sizeof(double) / 2;
	bacolr_(&bac->t, &tend, atol, rtol, &bac->ivp.neqs, &kcol,
		&bac->ivp.pint, &bac->nint, bac->x, (int *) &bac->mflag,
		bac->rpar.iov_base, &lrp, bac->ipar.iov_base, &lip,
		bac->bd.cpar.iov_base, &lcp, bac->y, &idid);
	    break;
#endif
#ifdef MPT_BACOL_DASSL
	    case 'd': case 'D':
	bacol_(&bac->t, &tend, atol, rtol, &bac->ivp.neqs, &kcol,
		&bac->ivp.pint, &bac->nint, bac->x, (int *) &bac->mflag,
		bac->rpar.iov_base, &lrp, bac->ipar.iov_base, &lip, bac->y, &idid);
	    break;
#endif
	    default: errno = EBADR;
	    return MPT_ERROR(BadArgument);
	}
	bac->mflag.noinit = idid;
	if (idid < 0) {
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
