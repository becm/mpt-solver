/*!
 * generic user functions LIMEX solver instance
 */

#include "limex.h"

#include "../solver_daefcn.h"

static void limex_fcn(int *neq, int *nz, double *t, double *y, double *f, double *b, int *ir, int *ic, int *info)
{
	const MPT_SOLVER_STRUCT(limex_param) *lp = (void *) neq;
	const MPT_SOLVER_STRUCT(limex) *lx = (void *) neq;
	const MPT_IVP_STRUCT(daefcn) *fcn = lp->ufcn;
	const MPT_IVP_STRUCT(pdefcn) *pde = (void *) fcn;
	int res;
	
	/* calcualte PDE */
	if ((res = pde->fcn(pde->par, *t, y, f, &lx->ivp)) < 0) {
		*info = res;
		return;
	}
	/* identity matrix */
	if (!fcn->mas.fcn) {
		int i, n = neq[0];
		for (i = 0; i < n; ++i) {
			ir[i] = i + 1;
		}
		for (i = 0; i < n; ++i) {
			ic[i] = i + 1;
		}
		for (i = 0; i < n; ++i) {
			b[i] = 1.;
		}
		*nz = n;
	}
	/* partial mass matrices */
	else {
		int i, max, neqs, pint;
		
		neqs = lx->ivp.neqs;
		pint = lx->ivp.pint;
		
		max = neq[0] * neq[0];
		*nz = 0;
		for (i = 0; i <= pint; ++i) {
			int j, n;
			
			*ir = max;
			*ic = i;
			
			if ((n = fcn->mas.fcn(fcn->mas.par, *t, y, b, ir, ic)) < 0) {
				*info = n;
				return;
			}
			for (j = 0; j < n; ++j) {
				++ir[j];
			}
			for (j = 0; j < n; ++j) {
				++ic[j];
			}
			*nz += n;
			ir += n;
			ic += n;
			b  += n;
			
			y += neqs;
		}
	}
	*info = 0;
}

static void limex_jac(int *neq, double *t, double *y, double *ys, double *jac, int *ldjac, int *ml, int *mu, int *banded, int *info)
{
	const MPT_SOLVER_STRUCT(limex_param) *lp = (void *) neq;
	const MPT_IVP_STRUCT(daefcn) *fcn = lp->ufcn;
	int ld;
	
	(void) ys;
	(void) ml;
	
	if (!*banded) {
		ld = *ldjac;
	} else {
		/* Jac(i+k,j), k = mu + 1 - j */
		jac += *mu;
		ld = *ldjac - 1;
	}
	*info = fcn->jac.fcn(fcn->jac.par, *t, y, jac, ld);
}

extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *lx, MPT_IVP_STRUCT(daefcn) *ufcn, int type, const void *par)
{
	int ret;
	
	if ((ret = _mpt_solver_daefcn_set(lx->ivp.pint, ufcn, type, par)) < 0) {
		return ret;
	}
	lx->fcn = 0;
	lx->jac = 0;
	if (!(lx->ufcn = ufcn)) {
		return 0;
	}
	if (ufcn->jac.fcn) {
		lx->jac = limex_jac;
	}
	if (ufcn->rside.fcn) {
		lx->fcn = limex_fcn;
	}
	return ret;
}

