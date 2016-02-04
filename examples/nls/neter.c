/*!
 * overdetemined NLEQ example with solver interface
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <mpt/array.h>
#include <mpt/solver.h>

/*  model for p1 * exp(p2 * x) by Neter (1983) */
static double model(double x, const double *p)
{
	return p[0] * exp(p[1]* x);
}

/* calculate residuals */
static int res(void *udata, const double *x, double *r, const int *lr)
{
	const double *u = udata;
	int i, nres = *lr;
	
	for (i = 0; i < nres; i++) {
		r[i] = u[2*i+1] - model(u[2*i], x);
	}
	return 0;
}

/* calculate jacobian */
static int deriv(void *udata, const double *x, double *jac, const int *lj, const double *res)
{
	const double *u = udata;
	int i, nres, ljac;
	
	(void) res;
	
	ljac = lj[0]; /* lenth of jacobian row */
	nres = lj[1]; /* number of residuals */
	i    = lj[2]; /* number of parameters */
	
	for (i = 0; i < nres ; i++) {
		double t = u[2*i];
		jac[i]      = - exp(x[1] * t);
		jac[i+ljac] = - x[0] * t *exp(x[1] * t);
	}
	return 0;
}

int user_init(MPT_SOLVER_STRUCT(nlsfcn) *usr, MPT_SOLVER_STRUCT(data) *sd)
{
	double *u;
	
	u = mpt_data_grid(sd);
	u += sd->nval;
	
	usr->res = res;
	usr->jac = deriv;
	usr->rpar = usr->jpar = u;
	
	return 2;
}

