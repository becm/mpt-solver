/*!
 * generic user function wrapper for PORT N2 instance
 */

#include "portn2.h"

#include "module_functions.h"

static void portn2_fcn(const int *n, const int *p, const double *x, int *nf, double *r, int *ui, double *ur, void (*uf)())
{
	MPT_NLS_STRUCT(functions) *ufcn = (void *) ui;
	int ld[2];
	
	(void) ur;
	(void) uf;
	
	ld[0] = *n;
	ld[1] = *p;
	
	if (ufcn->res.fcn(ufcn->res.par, x, r, ld) < 0) {
		*nf = 0;
	}
}

static void portn2_jac(const int *n, const int *p, const double *x, int *nf, double *jac, int *ui, double *ur, void (*uf)())
{
	MPT_NLS_STRUCT(functions) *ufcn = (void *) ui;
	int ld[3];
	
	(void) ur;
	(void) uf;
	
	ld[0] = *n;
	ld[1] = *n;
	ld[2] = *p;
	
	if (ufcn->jac.fcn(ufcn->jac.par, x, jac, ld, 0) < 0) {
		*nf = 0;
	}
}

extern int mpt_portn2_ufcn(MPT_SOLVER_STRUCT(portn2) *n2, MPT_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if (n2->nls.nval > n2->nls.nres) {
		return MPT_ERROR(BadArgument);
	}
	if ((ret = mpt_solver_module_ufcn_nls(&n2->nls, ufcn, type, ptr)) < 0) {
		return ret;
	}
	if (!(n2->ui = (void *) ufcn)) {
		n2->res.res = 0;
		n2->jac.jac = 0;
		return ret;
	}
	if (ufcn->res.fcn) {
		n2->res.res = portn2_fcn;
	} else {
		n2->res.res = 0;
	}
	if (ufcn->jac.fcn) {
		n2->jac.jac = portn2_jac;
		n2->nd =  0;
	} else {
		n2->jac.jac = 0;
		n2->nd = -1;
	}
	return ret;
}

