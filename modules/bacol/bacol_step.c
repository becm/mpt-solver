/*!
 * general wrapper for BACOL step and output
 */

#include <errno.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_step(MPT_SOLVER_STRUCT(bacol) *data, double *yout, double tend, double *xout)
{
	int	val, dim;
	
	if (data->mflag.noinit < 0) {
		errno = EINVAL; return -1;
	}
	dim = data->ivp.pint + 1;
	
	/* internal grid initialisation */
	if (!data->mflag.noinit) {
		if (!xout) {
			errno = EFAULT; return -1;
		}
		if (data->xinit) {
			val = data->xinit(dim, xout, data->nintmx/2, data->x, data->nint);
			if (val < 1 || val > data->nintmx/2) {
				errno = ERANGE; return -1;
			}
			data->nint = val;
		}
		else if (dim > data->nintmx/2) {
			errno = ERANGE; return -1;
		}
		else {
			(void) memcpy(data->x, xout, dim*sizeof(*data->x));
			data->nint = data->ivp.pint;
		}
	}
	val = mpt_bacol_sstep(data, tend);
	
	if (val < 0)
		return val;
	
	/* leave if no grid data */
	if (!xout && !yout)
		return 0;
	
	/* generate y-values for grid */
	if ((val = mpt_bacol_values(data, xout, 0, yout)) < 0)
		return val;
	
	return  (val != dim) ? val : 0;
}
