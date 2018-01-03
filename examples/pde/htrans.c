/*!
 * 1-D heat transfer simulation
 */

#include "solver_run.h"

static int rfcn(void *udata, double t, const double *u, double *f, double x, double *d, double *v)
{
	(void) udata; (void) t; (void) u; (void) x;
	
	d[0] = 2e-5;
	v[0] = 0;
	f[0] = 0;
	
	return 0;
}
/* right side calculation */
static int rs_pde(void *udata, double t, const double *y, double *f, const MPT_IVP_STRUCT(parameters) *ivp)
{
	const double *yr, *grid;
	double *fr, dx, diff, vx;
	int npde, nint;
	
	npde = ivp->neqs;
	nint = ivp->pint;
	grid = ivp->grid;
	
	yr = y + nint * npde;
	fr = f + nint * npde;
	
	/* inner nodes: use central differences */
	mpt_residuals_cdiff(udata, t, y, f, ivp, rfcn);
	
	/* constant dirichlet left boundary */
	f[0] = 0.;
	
	/* insulated right boundary */
	dx = grid[nint] - grid[nint-1];
	rfcn(udata, t, yr, fr, grid[nint], &diff, &vx);
	
	fr[0] += vx * yr[0] - diff * (yr[0] - yr[-1]) / dx;
	
	return 0;
}
/* setup solver for PDE run */
static int htrans_init(const MPT_INTERFACE(metatype) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *out)
{
	MPT_IVP_STRUCT(pdefcn) usr = MPT_IVP_PDE_INIT;
	int ret, npde = 1;
	
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
	cl  = mpt_client_ivp(htrans_init);
	return solver_run(cl);
}
