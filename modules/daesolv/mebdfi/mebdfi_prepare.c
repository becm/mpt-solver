/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <stdlib.h>
#include <errno.h>

#include "mebdfi.h"

extern int mpt_mebdfi_prepare(MPT_SOLVER_STRUCT(mebdfi) *data, int neqs, int pdim)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	int liw, lrw, *iwk;
	double *yp;
	
	if (neqs < 1 || pdim < 0) {
		errno = EINVAL;
		return -2;
	}
	ivp = &data->ivp;
	pdim = (ivp->pint = pdim) + 1;
	neqs = (ivp->neqs = neqs) * pdim;
	
	/* vector tolerances in sufficent dimension */
	if (neqs < 2 || (!data->rtol.base && !data->atol.base)) {
		pdim = 0;
	}
	if (mpt_vecpar_cktol(&data->atol, ivp->neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	if (mpt_vecpar_cktol(&data->rtol, ivp->neqs, pdim, __MPT_IVP_RTOL) < 0) {
		return -1;
	}
	/* set banded matrix parameters */
	if (!data->jbnd) {
		data->mbnd[3] = neqs;
	}
	else {
		data->mbnd[2] = data->mbnd[0] + data->mbnd[1] + 1;
		data->mbnd[3] = 2*data->mbnd[0] + data->mbnd[1] + 1;
	}
	
	liw	= neqs + 14;
	/* ignore wrong comments in mebdfi.f and set ACTUALLY needed size */
	lrw	= (32 + 2 * data->mbnd[3]) * neqs + 2;
	
	if (!(iwk = mpt_vecpar_alloc(&data->iwork, liw, sizeof(int)))) {
		return -1;
	}
	if (!mpt_vecpar_alloc(&data->rwork, lrw, sizeof(double))) {
		return -1;
	}
	pdim = data->yp.iov_len / sizeof(double);
	
	if (!(yp = mpt_vecpar_alloc(&data->yp, neqs, sizeof(double)))) {
		return -1;
	}
	for ( ; pdim < neqs; pdim++) yp[pdim] = 0.0;
	
	data->state = 1;	/* initial call for mebdfi() */
	
	/* reset counter on reinitialisation */
	for (liw = 0; liw < 7; liw++) iwk[4+liw] = 0;
	
	return neqs;
}
