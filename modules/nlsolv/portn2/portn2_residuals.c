/*!
 * call wrapper to PORT N2 for problem parameter
 */

#include "portn2.h"

extern const double *mpt_portn2_residuals(const MPT_SOLVER_STRUCT(portn2) *n2)
{
	const MPT_SOLVER_NLS_STRUCT(parameters) *nl = &n2->nls;
	double *p, *r;
	int nf = -1;
	
	if (!n2->res.res || !(p = n2->pv.iov_base)) {
		return 0;
	}
	if (((int *) n2->iv.iov_base)[0] < 3) {
		return 0;
	}
	/* index of residual position start
	 *  R = 61  -> R1 = IV(R) -> CALCR(..., V(R1)); */
	r = n2->rv.iov_base;
	r += ((int *) n2->iv.iov_base)[60] - 1;
	if (n2->nd > 0) {
		int nd1 = nl->nres, n1 = 0, nr = nl->nres;
		n2->res.pres(&nl->nres, &nd1, &n1, &nr, &nl->nval, p, &nf, r, n2->ui, n2->ur, n2->uf);
	} else {
		n2->res.res(&nl->nres, &nl->nval, p, &nf, r, n2->ui, n2->ur, n2->uf);
	}
	if (!nf) {
		return 0;
	}
	return r;
}
