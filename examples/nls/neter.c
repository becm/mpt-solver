/*!
 * overdetemined NLEQ example with solver interface
 */

#include <math.h>

#include "solver_run.h"

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

int user_init(MPT_SOLVER(generic) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *log)
{
	MPT_NLS_STRUCT(functions) usr = MPT_NLSFCN_INIT;
	double *u;
	int ret;
	
	u = mpt_solver_data_grid(sd);
	
	usr.res.fcn = res;
	usr.jac.fcn = deriv;
	
	usr.res.par = usr.jac.par = u;
	
	if ((ret = mpt_init_nls(sol, &usr, sd, log)) < 0) {
		return ret;
	}
	return 2;
}

