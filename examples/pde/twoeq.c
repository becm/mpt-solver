#include <math.h>

#include <mpt/array.h>
#include <mpt/message.h>

#include <mpt/solver.h>

static double *param, *grid;

static int rfcn(int n, double t, const double *u, double x, double *f, double *d, double *v)
{
	double g, fg;
	
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
static int rs_pde(void *udata, const double *t, const double *y, double *f)
{
	const MPT_SOLVER_STRUCT(ivppar) *ivp = udata;
	const double *yr;
	double *fr, dx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	
	yr = y + npde * nint;
	fr = f + npde * nint;
	
	/* inner discretization (use central differences) */
	mpt_residuals_cdiff(rfcn, *t, grid, nint+1, y, npde, f);
	
	/* left boundary: neumann, dirichlet */
	dx = grid[1] - grid[0];
	f[0] = 2*(y[npde] - y[0])/dx/dx;
	f[1] = 0.;
	
	/* right boundary: dirichlet,neumann */
	dx = grid[nint] - grid[nint-1];
	fr[0] = 0.;
	fr[1] = 2*(yr[1-npde] - yr[1])/dx/dx;
	
	return 0;
}

/* set user functions for PDE step */
extern int user_init(MPT_SOLVER_STRUCT(ivpfcn) *usr, MPT_SOLVER_STRUCT(data) *sd, MPT_INTERFACE(output) *out)
{
	int npar, npde = 2;
	
	usr->fcn = rs_pde;
	
	param = mpt_data_param(sd);
	grid  = mpt_data_grid (sd, npde);
	
	if ((npar = sd->npar) < 2) {
		mpt_output_log(out, __func__, MPT_ENUM(LogError), "%s (npar=%d)", "missing parameters", npar);
	}
	
	return npde;
}

