/*!
 * call wrapper to PORT N2 for problem parameter
 */

#include <errno.h>

#include "portn2.h"

extern int mpt_portn2_solve(MPT_SOLVER_STRUCT(portn2) *n2)
{
	MPT_SOLVER_STRUCT(nlspar) *nl = &n2->nls;
	const double *bnd;
	double *p;
	int liv, lrv;
	
	if (!n2->res.res || !(p = n2->pv.iov_base)) {
		return MPT_ERROR(BadArgument);
	}
	if (((int *) n2->iv.iov_base)[0] < 0) {
		return MPT_ERROR(BadArgument);
	}
	bnd = p + n2->nls.nval;
	liv = n2->iv.iov_len / sizeof(int);
	lrv = n2->rv.iov_len / sizeof(double);
	
	
	if (!n2->bnd) {
		if (n2->nd < 0 || !n2->jac.jac) {
			dn2f_(&nl->nres, &nl->nval, p, n2->res.res,
			      n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			      n2->ui, n2->ur, n2->uf);
		}
		else if (!n2->nd) {
			dn2g_(&nl->nres, &nl->nval, p, n2->res.res,
			      n2->jac.jac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base, 
			      n2->ui, n2->ur, n2->uf);
		} else {
			dn2p_(&nl->nres, &n2->nd, &nl->nval, p, n2->res.pres,
			      n2->jac.pjac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			      n2->ui, n2->ur, n2->uf);
		}
	} else {
		if (n2->nd < 0 || !n2->jac.jac) {
			dn2fb_(&nl->nres, &nl->nval, p, bnd, n2->res.res,
			       n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			       n2->ui, n2->ur, n2->uf);
		}
		else if (!n2->nd) {
			dn2gb_(&nl->nres, &nl->nval, p, bnd, n2->res.res,
			       n2->jac.jac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			       n2->ui, n2->ur, n2->uf);
		} else {
			dn2pb_(&nl->nres, &n2->nd, &nl->nval, p, bnd, n2->res.pres,
			       n2->jac.pjac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			       n2->ui, n2->ur, n2->uf);
		}
	}
	
	if (((int *) n2->iv.iov_base)[0] > 6) {
		return 1;
	}
	
	if (((int *) n2->iv.iov_base)[0] > 14) {
		return MPT_ERROR(BadArgument);
	}
	return 0;
}

