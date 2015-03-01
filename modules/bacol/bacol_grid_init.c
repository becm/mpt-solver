/*!
 * initialize BACOL solver grid
 */

#include <string.h>

#include "bacol.h"

extern int mpt_bacol_grid_init(int dim, const double *val, int nintmx, double *x, int nint)
{
	double	min, max;
	
	if ( dim < 1 || nint < 1 )
		return -1;
	
	if ( nint > nintmx )
		nint = nintmx;
	
	min = (dim > 0) ? val[0] : 0.;
	max = (dim > 1) ? val[dim-1] : min + 1.;
	
	/* dimension less or equal to initial intervals is equal */
	if ( dim <= (nint + 1) ) {
		(void) memcpy(x, val, dim * sizeof(*x));
		nint = dim - 1;
	}
	/* more data than initial intervals */
	else {
		const double	*curr = val;
		double	sum;
		int	i, j, fakt, parts, left, post, pre, merge;
		
		fakt = 2;
		
		/* dimension my not superseed mergable area */
		while ( nint * fakt < dim )
			fakt++;
		
		parts = dim / fakt;
		
		/* merged, left and max. overlapping points may not exceed total */
		while ( ((merge = parts*fakt) + (left = nint+1-parts) - 2 * (fakt/3)) > dim )
			parts--;
		
		/* original points after/before merged area */
		post = left/2;
		pre  = left - post;
		
		for ( i = 1 ; i < pre ; i++ )
			x[i] = curr[i];
		
		/* center area to be merged (round up) */
		curr += (dim - merge + 1)/2;
		
		for ( i = 0 ; i < parts ; i++ ) {
			for ( j = 0 , sum = 0. ; j < fakt ; j++ )
				sum += curr[i*fakt+j];
			
			x[pre + i] = sum/fakt;
		}
		curr = val + dim - post;
		for ( i = pre + i ; i < nint ; i++ )
			x[i] = *(curr++);
	}
	x[0]    = min;
	x[nint] = max;
	
	return nint;
}

