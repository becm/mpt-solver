/*!
 * calculate central difference residuals for inner nodes.
 */

#include <errno.h>
#include <string.h>

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

extern int mpt_residuals_cdiff(MPTRsideFcn rfcn, double t, const double *grid, int np, const double *y, int npde, double *f)
{
	const double *left;
	double dx, *diff, *vx;
	int i;
	
	/* inner nodes required */
	if ((np -= 2) <= 0) {
		errno = ERANGE;
		return -2;
	}
	
	diff = f;  /* initial temporary data */
	vx   = f + npde;
	
	left = y;
	y   += npde;
	
	/* calculate non-boundary nodes */
	while (np--) {
		const double *right = y + npde;
		
		f = vx;  /* advance temporary data */
		vx += npde;
		
		if ((i = rfcn(npde, t, y, grid[1], f, diff, vx)) < 0) {
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

