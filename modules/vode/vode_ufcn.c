/*!
 * generic user functions dVODE solver instance
 */

#include <stdlib.h>
#include <errno.h>

#include "vode.h"

static void vode_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(odefcn) *ode = (void *) ipar;
	MPT_SOLVER_STRUCT(vode) *vd = (void *) rpar;
	int ret;
	(void) neq;
	
	if (vd->ivp.pint) {
		MPT_SOLVER_STRUCT(pdefcn) *pde = (void *) ipar;
		ret = pde->fcn(pde->param, *t, y, f, &vd->ivp, pde->grid, pde->rside);
	} else {
		ret = ode->fcn(ode->param, *t, y, f);
	}
	if (ret < 0) {
		abort();
	}
}

static void vode_jac(int *neq, double *t, double *y, int *ml, int *mu, double *jac, int *ljac, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(odefcn) *ufcn = (void *) ipar;
	int ld;
	
	(void) rpar;
	
	if (*ml >= *neq) {
		ld = *ljac;
	} else {
		/* irow = i - j + MU + 1 */
		jac += *mu;
		ld   =  *ljac - 1;
	}
	if (ufcn->jac(ufcn->param, *t, y, jac, ld) < 0) {
		abort();
	}
}

extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *vd, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
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

