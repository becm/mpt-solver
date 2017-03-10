/*!
 * generic user functions dVODE solver instance
 */

#include <stdlib.h>

#include "vode.h"

static void vode_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	const MPT_SOLVER_IVP_STRUCT(functions) *ufcn = (void *) ipar;
	const MPT_SOLVER_STRUCT(vode) *vd = (void *) rpar;
	int ret;
	(void) neq;
	
	ret = ((MPT_SOLVER_STRUCT(pdefcn) *) ufcn)->fcn(ufcn->dae.param, *t, y, f, &vd->ivp, ufcn->grid, ufcn->rside);
	if (ret < 0) {
		abort();
	}
}

static void vode_jac(int *neq, double *t, double *y, int *ml, int *mu, double *jac, int *ljac, double *rpar, int *ipar)
{
	const MPT_SOLVER_IVP_STRUCT(functions) *ufcn = (void *) ipar;
	int ld;
	
	(void) rpar;
	
	if (*ml >= *neq) {
		ld = *ljac;
	} else {
		/* irow = i - j + MU + 1 */
		jac += *mu;
		ld   =  *ljac - 1;
	}
	if (ufcn->dae.jac(ufcn->dae.param, *t, y, jac, ld) < 0) {
		abort();
	}
}

extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *vd, const MPT_SOLVER_IVP_STRUCT(functions) *ufcn)
{
	if (!ufcn || !ufcn->dae.fcn) {
		return MPT_ERROR(BadArgument);
	}
	vd->jac = ufcn->dae.jac ? vode_jac : 0;
	vd->fcn = vode_fcn;
	vd->ipar = (int *) ufcn;
	vd->rpar = (double *) vd;
	
	return 0;
}

