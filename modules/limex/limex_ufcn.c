/*!
 * generic user functions LIMEX solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "limex.h"

struct _limexParam
{
	int neq, pdim;
	const MPT_SOLVER_STRUCT(daefcn) *ufcn;
};
static void limex_fcn(int *neq, int *nz, double *t, double *y, double *f, double *b, int *ir, int *ic, int *info)
{
	const MPT_SOLVER_STRUCT(daefcn) *dae = ((struct _limexParam *) neq)->ufcn;
	int n = neq[0], nint = neq[1];
	
	/* calcualte PDE */
	if (nint) {
		MPT_SOLVER_STRUCT(pdefcn) *pde = (void *) dae;
		MPT_SOLVER_STRUCT(ivppar) ivp;
		ivp.neqs = *neq / nint;
		ivp.pint = nint - 1;
		if (!(n = pde->fcn(pde->param, *t, y, f, &ivp, pde->grid, pde->rside))) {
			*info = 0;
			return;
		}
	}
	/* partial mass matrices */
	if (dae->mas) {
		int i;
		if ((n = dae->mas(dae->param, *t, y, b, ir, ic)) < 0) {
			*info = n;
			return;
		}
		for (i = 0; i < n; ++i) {
			++ir[i];
		}
		for (i = 0; i < n; ++i) {
			++ic[i];
		}
		*nz = n;
	}
	/* identity matrix */
	else {
		int i;
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
	if (!(n = dae->fcn(dae->param, *t, y, f))) {
		*info = 0;
		return;
	}
	*info = (n < 0) ? -2 : -1;
}

static void limex_jac(int *neq, double *t, double *y, double *ys, double *jac, int *ldjac, int *ml, int *mu, int *banded, int *info)
{
	const MPT_SOLVER_STRUCT(daefcn) *dae = ((struct _limexParam *) neq)->ufcn;
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
	*info = dae->jac(dae->param, *t, y, jac, ld);
}

extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *lx, const MPT_SOLVER_STRUCT(daefcn) *dae)
{
	if (!dae || !dae->fcn) {
		return MPT_ERROR(BadArgument);
	}
	
	lx->jac  = dae->jac && !lx->ivp.pint ? limex_jac : 0;
	lx->fcn  = limex_fcn;
	lx->ufcn = dae;
	
	return 0;
}

