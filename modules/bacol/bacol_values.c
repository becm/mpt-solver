/*!
 * create output values for BACOL descriptor
 */

#include <stdlib.h>
#include <errno.h>

#include "bacol.h"

extern int mpt_bacol_values(MPT_SOLVER_STRUCT(bacol) *data, double *xout, int deriv, double *yout)
{
	int	wlen, dim = data->ivp.pint + 1;
	
	/* leave if no grid data */
	if (!xout) {
		errno = EFAULT; return -1;
	}
	
	/* adapt grid points to integrator intervals */
	if (data->xgrid)
		if ( (dim = data->xgrid(data->ivp.last, data->nint, data->x, dim, xout)) < 0 )
			return dim;
	
	/* generate y-values for grid */
	if (yout) {
		int	kcol = data->kcol;
		/* check output work data availability */
		wlen	= (kcol * MPT_BACOL_NCONTI) * (1 + deriv);
		wlen	+= kcol * (data->nint + 1) + 2 * MPT_BACOL_NCONTI;
		
		if ( !mpt_vecpar_alloc(&data->owrk, wlen, sizeof(double)) )
			return -2;
		
		values_(&kcol, xout, &data->nint, data->x, &data->ivp.neqs, &dim,
			&deriv, yout, data->y, data->owrk.iov_base);
	}
	return dim;
}


