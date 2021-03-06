/*!
 * PDE example with single equotation
 */

#include "solver_run.h"

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
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_IVP_STRUCT(parameters) *ivp)
{
	const double *yr, *grid;
	double *fr, dx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	grid = ivp->grid;
	
	yr = y + nint * npde;  /* last node start */
	fr = f + nint * npde;
	
	/* inner node residuals: use central differences */
	mpt_residuals_cdiff(udata, t, y, f, ivp, rfcn);
	
	/* constant dirichlet left boundary */
	f[0] = 0.;
	
	/* neumann right boundary */
	dx = grid[nint] - grid[nint - 1];
	fr[0] = 2 * (yr[-1] - yr[0]) / dx / dx;
	
	return 0;
}
/* solver/client setup for PDE run */
static int diff_init(MPT_INTERFACE(convertable) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_IVP_STRUCT(pdefcn) usr = MPT_IVP_PDE_INIT;
	static double diff = 0.094;
	int ret, npde = 1;
	
	/* initialize values from solver configuration */
	usr.par = mpt_solver_data_param(sd);
	if (!usr.par) {
		usr.par = &diff;
	}
	usr.fcn = rs_pde;
	if ((ret = mpt_init_pde(sol, &usr, npde, &sd->val, out)) < 0) {
		return ret;
	}
	return npde;
}
int main(int argc, char * const argv[])
{
	MPT_INTERFACE(client) *cl;
	if (mpt_init(argc, argv) < 0) {
		return 1;
	}
	cl = mpt_client_ivp(diff_init);
	return solver_run(cl);
}
