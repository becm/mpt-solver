/*!
 * check/adapt PORT N2 memory sizes and parameters
 */

#include <stdlib.h>
#include <float.h>

#include "portn2.h"

extern int mpt_portn2_prepare(MPT_SOLVER_STRUCT(portn2) *n2, int nval, int nres)
{
	int liv, lrv;
	
	if (nval < 1 || nres < nval) {
		return MPT_ERROR(BadArgument);
	}
	
	if (!n2->jac.jac) {
		n2->nd = -1;
	}
	/* full jacobian (user or numerical) */
	if (n2->nd <= 0) {
		if (n2->bnd) {
			liv = 82 + 4*nres;
			lrv = 105 + nres*(nval + 2*nres + 21) + 2*nval;
		} else {
			liv = 82 + nres;
			lrv = 105 + nres*(nval + 2*nres + 17) + 2*nval;
		}
	}
	/* partial user jacobian */
	else {
		int nd = (n2->nd < nval) ? n2->nd : nval;
		liv = 82 + nres;
		lrv = 105 + nres*(17 + 2*nres) + (nres+1)*nd + nval;
	}
	
	if (!mpt_vecpar_alloc(&n2->rv, lrv, sizeof(double))) {
		return MPT_ERROR(BadOperation);
	}
	if (!mpt_vecpar_alloc(&n2->iv, liv, sizeof(int))) {
		return MPT_ERROR(BadOperation);
	}
	lrv = n2->pv.iov_len/sizeof(double);
	liv = n2->bnd ? 3*nval : nval;
	if (lrv < liv) {
		double *bnd;
		
		if (!(bnd = mpt_vecpar_alloc(&n2->pv, liv, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
		/* fill default initial parameters */
		for ( ; lrv < nval; lrv++) {
			bnd[lrv] = 0;
		}
		/* set remaining boundaries */
		if (n2->bnd) {
			for (lrv = 0; lrv < nval; lrv++) {
				bnd[nval+2*lrv]   = -DBL_MAX;
				bnd[nval+2*lrv+1] =  DBL_MAX;
			}
		}
	}
	n2->nls.nval = nval;
	n2->nls.nres = nres;
	
	return ((int *) n2->iv.iov_base)[0] = 0;
}
