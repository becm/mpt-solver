/*!
 * PDE example with two equotations
 */

#include <math.h>

#include "solver_ivp.h"

static double *param;

/*  non-boundery positions */
static int rfcn(void *udata, double t, const double *u, double *f, double x, double *d, double *v)
{
	double g, fg;
	
	(void) udata; (void) t; (void) x;
	
	d[0] = param[0];
	d[1] = param[1];
	
	v[0] = v[1] = 0;
	g = u[0] - u[1];
	
	fg = exp(param[2]*g) - exp(param[3]*g);
	
	f[0] = -fg;
	f[1] = fg;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_SOLVER_IVP_STRUCT(parameters) *ivp, const double *grid, MPT_SOLVER_IVP(Rside) rs)
{
	const double *yr;
	double *fr, dx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	yr = y + npde * nint;
	fr = f + npde * nint;
	
	/* inner discretization (use central differences) */
	mpt_residuals_cdiff(udata, t, y, f, ivp, grid, rs);
	
	/* left boundary: neumann, dirichlet */
	dx = grid[1] - grid[0];
	f[0] = 2*(y[npde] - y[0])/dx/dx;
	f[1] = 0.;
	
	/* right boundary: dirichlet, neumann */
	dx = grid[nint] - grid[nint-1];
	fr[0] = 0.;
	fr[1] = 2*(yr[1-npde] - yr[1])/dx/dx;
	
	return 0;
}

/* setup solver for PDE run */
extern int user_init(MPT_SOLVER(IVP) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_SOLVER_STRUCT(pdefcn) *usr;
	int npar, npde = 2;
	
	/* require minimal parameter count */
	if ((npar = sd->npar) < 2) {
		mpt_log(out, __func__, MPT_LOG(Error), "%s (npar=%d)",
		        "missing parameters", npar);
		return MPT_ERROR(BadValue);
	}
	param = mpt_solver_data_param(sd);
	
	if (!(usr = mpt_init_pde(sol, npde, sd->nval, out))
	    || !(usr->grid = mpt_solver_data_grid(sd))) {
		return MPT_ERROR(BadArgument);
	}
	usr->fcn = rs_pde;
	usr->rside = rfcn;
	
	
	return npde;
}

