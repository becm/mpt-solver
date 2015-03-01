/*!
 * check/adapt PORT N2 memory sizes and parameters
 */

#include <stdlib.h>
#include <float.h>
#include <errno.h>

#include "portn2.h"

extern int mpt_portn2_prepare(MPT_SOLVER_STRUCT(portn2) *n2, int nval, int nres)
{
	int	liv, lrv;
	
	if (nval < 1 || nres < nval) {
		errno = EINVAL;
		return -2;
	}
	
	if (!n2->jac.jac) {
		n2->nd = -1;
	}
	/* full jacobian (user or numerical) */
	if (n2->nd <= 0) {
		if (n2->bnd.iov_base && n2->bnd.iov_len/2/sizeof(double)) {
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
		return -1;
	}
	if (!mpt_vecpar_alloc(&n2->iv, liv, sizeof(int))) {
		return -1;
	}
	if (n2->bnd.iov_base && (lrv = n2->bnd.iov_len/sizeof(double)/2) < nval) {
		double *bnd;
		
		if (!(bnd = mpt_vecpar_alloc(&n2->bnd, 2*nval, sizeof(double)))) {
			return -1;
		}
		for ( ; lrv < nval; lrv++) {
			bnd[2*lrv]  = -DBL_MAX;
			bnd[2*lrv+1] = DBL_MAX;
		}
	}
	n2->nls.nval = nval;
	n2->nls.nres = nres;
	
	return ((int *) n2->iv.iov_base)[0] = 0;
}
