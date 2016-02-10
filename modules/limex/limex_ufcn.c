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
	const MPT_SOLVER_IVP_STRUCT(functions) *ufcn;
};
static void limex_fcn(int *neq, int *nz, double *t, double *y, double *f, double *b, int *ir, int *ic, int *info)
{
	const MPT_SOLVER_IVP_STRUCT(functions) *fcn = ((struct _limexParam *) neq)->ufcn;
	MPT_SOLVER_IVP_STRUCT(parameters) ivp;
	int res, nint = neq[1];
	
	ivp.neqs = neq[0] / (nint + 1);
	ivp.pint = nint;
	
	/* calcualte PDE */
	if ((res = ((const MPT_SOLVER_STRUCT(pdefcn) *) fcn)->fcn(fcn->dae.param, *t, y, f, &ivp, fcn->grid, fcn->rside)) < 0) {
		*info = res;
		return;
	}
	/* identity matrix */
	if (!fcn->dae.mas) {
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
		
		neqs = ivp.neqs;
		pint = ivp.pint;
		
		max = neq[0] * neq[0];
		*nz = 0;
		for (i = 0; i <= pint; ++i) {
			int j, n;
			
			*ir = max;
			*ic = i;
			
			if ((n = fcn->dae.mas(fcn->dae.param, *t, y, b, ir, ic)) < 0) {
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
	const MPT_SOLVER_IVP_STRUCT(functions) *fcn = ((struct _limexParam *) neq)->ufcn;
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
	*info = fcn->dae.jac(fcn->dae.param, *t, y, jac, ld);
}

extern int mpt_limex_ufcn(MPT_SOLVER_STRUCT(limex) *lx, const MPT_SOLVER_IVP_STRUCT(functions) *usr)
{
	if (!usr || !usr->dae.fcn) {
		return MPT_ERROR(BadArgument);
	}
	
	lx->fcn  = limex_fcn;
	lx->jac  = usr->dae.jac ? limex_jac : 0;
	lx->ufcn = usr;
	
	return 0;
}

