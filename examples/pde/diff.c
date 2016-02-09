#include <math.h>

#include <mpt/solver.h>

static int rfcn(void *udata, double t, const double *u, double *f, double x, double *d, double *v)
{
	double *param = udata;
	
	(void) t; (void) u; (void) x;
	
	d[0] = *param;
	v[0] = 0;
	f[0] = 0;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_SOLVER_STRUCT(ivppar) *ivp, const double *grid, MPT_SOLVER_TYPE(RsideFcn) rs)
{
	const double *yr;
	double *fr, dx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	yr = y + nint * npde;  /* last node start */
	fr = f + nint * npde;
	
	/* inner node residuals: use central differences */
	mpt_residuals_cdiff(udata, t, y, f, ivp, grid, rs);
	
	/* constant dirichlet left boundary */
	f[0] = 0.;
	
	/* neumann right boundary */
	dx = grid[nint] - grid[nint-1];
	fr[0] = 2*(yr[-1] - yr[0])/dx/dx;
	
	return 0;
}

/* set user functions for PDE step */
extern int user_init(MPT_SOLVER(IVP) *sol, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_SOLVER_STRUCT(pdefcn) *usr;
	static double diff = 0.094;
	int npde = 1;
	
	if (!(usr = mpt_init_pde(sol, npde, sd->nval, out))
	    || !(usr->grid = mpt_data_grid(sd))) {
		return MPT_ERROR(BadArgument);
	}
	usr->fcn = rs_pde;
	usr->rside = rfcn;
	
	/* initialize values from solver configuration */
	if (!(usr->param = mpt_data_param(sd))) {
		usr->param = &diff;
	}
	
	return npde;
}
