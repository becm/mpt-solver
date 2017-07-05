/*!
 * PDE example with two equotations
 */

#include <math.h>

#include "solver_ivp.h"

static double *param, damkohler, L = 1, a = 1, de = 20, R = 5;

static int rfcn(void *udata, double t, const double *u, double *f, double x, double *d, double *v)
{
	double fg;
	
	(void) udata; (void) t; (void) x;
	
	d[0] = 1;
	d[1] = 1/L;
	
	v[0] = v[1] = 0;
	
	fg = damkohler * u[0] * exp(-de/u[1]);
	
	f[0] = -fg;
	f[1] = a / L * fg;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_SOLVER_IVP_STRUCT(parameters) *ivp)
{
	const double *grid;
	double *fr, dx, fg;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	grid = ivp->grid;
	
	fr = f + npde * nint;
	
	/* inner residuals (central differences) */
	mpt_residuals_cdiff(udata, t, y, f, ivp, rfcn);
	
	/* left boundary: neumann */
	dx = grid[1] - grid[0];
	fg = damkohler * y[0] * exp(-de/y[1]);
	
	f[0] = 2 * (y[npde]     - y[0]) / dx / dx - fg;
	f[1] = 2 * (y[npde + 1] - y[1]) / dx / dx + a / L * fg;
	
	/* right boundary: dirichlet */
	fr[0] = 0;
	fr[1] = 0;
	
	return 0;
}

/* setup solver for PDE run */
extern int user_init(MPT_SOLVER(generic) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_SOLVER_IVP_STRUCT(pdefcn) pde;
	int ret, npde = 2;
	
	pde.fcn = rs_pde;
	if ((ret = mpt_init_pde(sol, &pde, npde, &sd->val, out)) < 0) {
		return MPT_ERROR(BadArgument);
	}
	param = mpt_solver_data_param(sd);
	
	switch (sd->npar) {
	    default: R = param[3];
	    case 3: de = param[2];
	    case 2: a  = param[1];
	    case 1: L  = param[0];
	    case 0:;
	}
	
	damkohler = R * exp(de) / (a * de);
	
	return npde;
}

