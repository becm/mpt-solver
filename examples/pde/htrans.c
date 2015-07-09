#include <math.h>

#include <mpt/solver.h>

static double *grid;

static int rfcn(int npde, double t, const double *u, double x, double *f, double *d, double *v)
{
	(void) npde; (void) t; (void) u; (void) x;
	
	d[0] = 2e-5;
	v[0] = 0;
	f[0] = 0;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, const double *t, const double *y, double *f)
{
	const MPT_SOLVER_STRUCT(ivppar) *ivp = udata;
	const double *yr;
	double *fr, dx, diff, vx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	yr = y + nint * npde;
	fr = f + nint * npde;
	
	/* inner nodes: use central differences */
	mpt_residuals_cdiff(rfcn, *t, grid, nint+1, y, npde, f);
	
	/* constant dirichlet left boundary */
	f[0] = 0.;
	
	/* insulated right boundary */
	dx = grid[nint] - grid[nint-1];
	rfcn(npde, *t, yr, grid[nint], fr, &diff, &vx);
	
	fr[0] += vx * yr[0] - diff * (yr[0] - yr[-1]) / dx;
	
	return 0;
}

/* set user functions for PDE step */
extern int user_init(MPT_SOLVER_STRUCT(ivpfcn) *usr, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(output) *out)
{
	int npde = 1;
	
	(void) out;
	
	usr->fcn = rs_pde;
	
	grid = mpt_data_grid(sd, npde);
	
	return npde;
}
