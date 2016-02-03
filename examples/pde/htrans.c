#include <math.h>

#include <mpt/solver.h>

static double *grid;

static int rfcn(void *udata, double t, const double *u, double *f, double x, double *d, double *v)
{
	(void) udata; (void) t; (void) u; (void) x;
	
	d[0] = 2e-5;
	v[0] = 0;
	f[0] = 0;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_SOLVER_STRUCT(ivppar) *ivp, const double *grid, MPT_SOLVER_TYPE(RsideFcn) rs)
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

/* set user functions for PDE step */
extern int user_init(MPT_SOLVER_STRUCT(pdefcn) *usr, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(output) *out)
{
	int npde = 1;
	
	(void) out;
	
	usr->fcn = rs_pde;
	usr->rside = rfcn;
	
	grid = mpt_data_grid(sd, npde);
	
	return npde;
}
