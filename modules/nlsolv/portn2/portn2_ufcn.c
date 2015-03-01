/*!
 * generic user function wrapper for PORT N2 instance
 */

#include <errno.h>

#include "portn2.h"

static void portn2_fcn(const int *n, const int *p, const double *x, int *nf, double *r, int *ui, double *ur, void (*uf)())
{
	MPT_SOLVER_STRUCT(nlsfcn) *ufcn = (MPT_SOLVER_STRUCT(nlsfcn) *) ui;
	int ld[2];
	
	(void) ur;
	(void) uf;
	
	ld[0] = *n;
	ld[1] = *p;
	
	if (ufcn->res(ufcn->rpar, x, r, ld) < 0)
		*nf = 0;
}

static void portn2_jac(const int *n, const int *p, const double *x, int *nf, double *jac, int *ui, double *ur, void (*uf)())
{
	MPT_SOLVER_STRUCT(nlsfcn) *ufcn = (MPT_SOLVER_STRUCT(nlsfcn) *) ui;
	int ld[3];
	
	(void) ur;
	(void) uf;
	
	ld[0] = *n;
	ld[1] = *n;
	ld[2] = *p;
	
	if (ufcn->jac(ufcn->jpar, x, jac, ld, 0) < 0) {
		*nf = 0;
	}
}

extern int mpt_portn2_ufcn(MPT_SOLVER_STRUCT(portn2) *n2, const MPT_SOLVER_STRUCT(nlsfcn) *ufcn)
{
	if (!ufcn->res) {
		errno = EINVAL;
		return -1;
	}
	n2->res.res = portn2_fcn;
	n2->jac.jac = ufcn->jac ? portn2_jac : 0;
	
	n2->nd = n2->jac.jac ? 0 : -1;
	n2->ui = (int *) ufcn;
	
	return 0;
}

