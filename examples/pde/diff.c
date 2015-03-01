#include <math.h>

#include <mpt/solver.h>

static double *grid, *param, diff;

static int rfcn(int npde, double t, const double *u, double x, double *f, double *d, double *v)
{
	d[0] = diff;
	v[0] = 0;
	f[0] = 0;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, const double *t, const double *y, double *f)
{
	const MPT_SOLVER_STRUCT(ivppar) *ivp = udata;
	const double *yr;
	double *fr, dx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	yr = y + nint * npde;	/* last node start */
	fr = f + nint * npde;
	
	/* inner node residuals: use central differences */
	mpt_residuals_cdiff(rfcn, *t, grid, nint+1, y, npde, f);
	
	/* constant dirichlet left boundary */
	f[0] = 0.;
	
	/* neumann right boundary */
	dx = grid[nint] - grid[nint-1];
	fr[0] = 2*(yr[-1] - yr[0])/dx/dx;
	
	return 0;
}

/* set user functions for PDE step */
extern int user_init(MPT_SOLVER_STRUCT(ivpfcn) *usr, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(output) *out)
{
	int npde = 1;
	
	usr->fcn = rs_pde;
	
	/* initialize values from solver configuration */
	grid  = mpt_data_grid (sd, npde);
	param = mpt_data_param(sd);
	
	diff = param ? param[0] : 0.094;
	
	return npde;
}
