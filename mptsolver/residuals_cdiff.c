/*!
 * calculate central difference residuals for inner nodes.
 */

#include "solver.h"

static void cdiff_step(int npde, const double *left, const double *curr, const double *right, double dx, double *f, double *diff, double *vx)
{
	double idx2, idxq, uxx, ux;
	int i;
	
	idx2 = 1.0 / dx;
	idxq = idx2 * idx2;
	idx2 /= 2;
	
	for (i = 0; i < npde; i++) {
		uxx = (right[i] - 2*curr[i] + left[i]) * idxq;
		ux  = (right[i] - left[i]) * idx2;
		
		f[i] += diff[i] * uxx - vx[i] * ux;
	}
}

/*!
 * \ingroup mptSolver
 * \brief central differences
 * 
 * Calculate central differences for PDE state.
 * 
 * \param      ctx    problem context (passed to user function)
 * \param      t     current time step
 * \param[in]  y     current PDE state [in
 * \param[out] f     current PDE state
 * \param      ivp   PDE count and grid intervals
 * \param      grid  PDE grid data
 * \param      rfcn  grid point residual function
 * 
 * \return argument or user function error
 */
extern int mpt_residuals_cdiff(void *ctx, double t, const double *y, double *f, const MPT_SOLVER_IVP_STRUCT(parameters) *ivp, const double *grid, MPT_SOLVER_IVP(Rside) rfcn)
{
	const double *left;
	double dx, *diff, *vx;
	int pts, i, npde;
	
	/* require PDE arguments */
	if (!ivp || (npde = ivp->neqs) < 1 || !rfcn || !grid) {
		return MPT_ERROR(BadArgument);
	}
	/* inner nodes required */
	if ((pts = ivp->pint - 1) <= 0) {
		return MPT_ERROR(BadArgument);
	}
	diff = f;  /* initial temporary data */
	vx   = f + npde;
	
	left = y;
	y   += npde;
	
	/* calculate non-boundary nodes */
	while (pts--) {
		const double *right = y + npde;
		
		f = vx;  /* advance temporary data */
		vx += npde;
		
		if ((i = rfcn(ctx, t, y, f, grid[1], diff, vx)) < 0) {
			return i;
		}
		/* average node distance */
		dx = (grid[2] - grid[0]) / 2;
		
		cdiff_step(npde, left, y, right, dx, f, diff, vx);
		
		left = y; /* advance position */
		y = right;
		grid++;
	}
	
	/* set boundary residuals to zero */
	for (i = 0; i < npde; i++) diff[i] = 0.;
	for (i = 0; i < npde; i++) vx[i] = 0.;
	
	return 0;
}

