/*!
 * 1-D heat transfer simulation
 */

#include "solver_ivp.h"

static int rfcn(void *udata, double t, const double *u, double *f, double x, double *d, double *v)
{
	(void) udata; (void) t; (void) u; (void) x;
	
	d[0] = 2e-5;
	v[0] = 0;
	f[0] = 0;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_SOLVER_IVP_STRUCT(parameters) *ivp, const double *grid, MPT_SOLVER_IVP(Rside) rs)
{
	const double *yr;
	double *fr, dx, diff, vx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	yr = y + nint * npde;
	fr = f + nint * npde;
	
	/* inner nodes: use central differences */
	mpt_residuals_cdiff(udata, t, y, f, ivp, grid, rs);
	
	/* constant dirichlet left boundary */
	f[0] = 0.;
	
	/* insulated right boundary */
	dx = grid[nint] - grid[nint-1];
	rs(0, t, yr, fr, grid[nint], &diff, &vx);
	
	fr[0] += vx * yr[0] - diff * (yr[0] - yr[-1]) / dx;
	
	return 0;
}

/* setup solver for PDE run */
extern int user_init(MPT_SOLVER(IVP) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_SOLVER_STRUCT(pdefcn) *usr;
	int npde = 1;
	
	if (!(usr = mpt_init_pde(sol, npde, sd->nval, out))
	    || !(usr->grid = mpt_solver_data_grid(sd))) {
		return MPT_ERROR(BadArgument);
	}
	usr->fcn = rs_pde;
	usr->rside = rfcn;
	
	return npde;
}
