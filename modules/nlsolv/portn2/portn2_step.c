/*!
 * call wrapper to PORT N2 for problem parameter
 */

#include <errno.h>

#include "portn2.h"

extern int mpt_portn2_step(MPT_SOLVER_STRUCT(portn2) *n2, double *x, double *fvec)
{
	MPT_SOLVER_STRUCT(nlspar) *nl = &n2->nls;
	int liv, lrv;
	
	if (!n2->res.res || !x) {
		errno = EFAULT;
		return -1;
	}
	if (((int *) n2->iv.iov_base)[0] < 0) {
		errno = EINVAL;
		return -1;
	}
	
	liv = n2->iv.iov_len / sizeof(int);
	lrv = n2->rv.iov_len / sizeof(double);
	
	if (!n2->bnd.iov_base) {
		if (n2->nd < 0 || !n2->jac.jac) {
			dn2f_(&nl->nres, &nl->nval, x, n2->res.res,
			      n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			      n2->ui, n2->ur, n2->uf);
		}
		else if (!n2->nd) {
			dn2g_(&nl->nres, &nl->nval, x, n2->res.res,
			      n2->jac.jac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base, 
			      n2->ui, n2->ur, n2->uf);
		} else {
			dn2p_(&nl->nres, &n2->nd, &nl->nval, x, n2->res.pres,
			      n2->jac.pjac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			      n2->ui, n2->ur, n2->uf);
		}
	} else {
		if (n2->nd < 0 || !n2->jac.jac) {
			dn2fb_(&nl->nres, &nl->nval, x, n2->bnd.iov_base, n2->res.res,
			       n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			       n2->ui, n2->ur, n2->uf);
		}
		else if (!n2->nd) {
			dn2gb_(&nl->nres, &nl->nval, x, n2->bnd.iov_base, n2->res.res,
			       n2->jac.jac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			       n2->ui, n2->ur, n2->uf);
		} else {
			dn2pb_(&nl->nres, &n2->nd, &nl->nval, x, n2->bnd.iov_base, n2->res.pres,
			       n2->jac.pjac, n2->iv.iov_base, &liv, &lrv, n2->rv.iov_base,
			       n2->ui, n2->ur, n2->uf);
		}
	}
	
	if (((int *) n2->iv.iov_base)[0] > 8) {
		errno = EINVAL;
		return -1;
	}
	if (fvec) {
		if (n2->nd > 0) {
			int nd1 = nl->nres, n1 = 0, nr = nl->nres;
			n2->res.pres(&nl->nres, &nd1, &n1, &nr, &nl->nval, x, &liv, fvec, n2->ui, n2->ur, n2->uf);
		} else {
			n2->res.res(&nl->nres, &nl->nval, x, &liv, fvec, n2->ui, n2->ur, n2->uf);
		}
	}
	return 0;
}

