#include <math.h>

#include <mpt/array.h>
#include <mpt/solver.h>

static double *param, *grid, damkohler, L = 1, a = 1, de = 20, R = 5;

static int rfcn(int n, double t, const double *u, double x, double *f, double *d, double *v)
{
	double fg;
	
	(void) n; (void) t; (void) x;
	
	d[0] = 1;
	d[1] = 1/L;
	
	v[0] = v[1] = 0;
	
	fg = damkohler * u[0] * exp(-de/u[1]);
	
	f[0] = -fg;
	f[1] = a / L * fg;
	
	return 0;
}

/* solver right side calculation */
static int rs_pde(void *udata, const double *t, const double *y, double *f)
{
	const MPT_SOLVER_STRUCT(ivppar) *ivp = udata;
	double *fr, dx, fg;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	fr = f + npde * nint;
	
	/* inner residuals (central differences) */
	mpt_residuals_cdiff(rfcn, *t, grid, nint+1, y, npde, f);
	
	/* left boundary: neumann */
	dx = grid[1]-grid[0];
	fg = damkohler * y[0] * exp(-de/y[1]);
	
	f[0] = 2*(y[npde] - y[0])/dx/dx - fg;
	f[1] = 2*(y[npde+1] - y[1])/dx/dx + a / L * fg;
	
	/* right boundary: dirichlet */
	fr[0] = 0;
	fr[1] = 0;
	
	return 0;
}

/* set user functions for PDE step */
extern int user_init(MPT_SOLVER_STRUCT(ivpfcn) *usr, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(output) *out)
{
	int npde = 2;
	
	(void) out;
	
	usr->fcn = rs_pde;
	
	param = mpt_data_param(sd);
	grid  = mpt_data_grid (sd, npde);
	
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

