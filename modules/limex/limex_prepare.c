/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <string.h>
#include <errno.h>

#include "limex.h"

extern int mpt_limex_prepare(MPT_SOLVER_STRUCT(limex) *data, int neqs, int pdim)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	double *ys;
	int *ipos;
	
	if (neqs < 1 || pdim < 0) {
		errno = EINVAL;
		return -2;
	}
	
	ivp  = &data->ivp;
	pdim = (ivp->pint = pdim) + 1;
	neqs = (ivp->neqs = neqs) * pdim;
	
	if (data->iopt[7] < 0) data->iopt[7] = ivp->neqs;
	if (data->iopt[8] < 0) data->iopt[8] = ivp->neqs;
	
	/* vector tolerances in sufficent dimension */
	if (ivp->neqs < 2 || (!data->rtol.base && !data->atol.base)) pdim = 0;
	
	if (mpt_vecpar_cktol(&data->atol, neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	if (mpt_vecpar_cktol(&data->rtol, neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	pdim = data->ipos.iov_len / sizeof(int);
	if (!(ipos = mpt_vecpar_alloc(&data->ipos, neqs, sizeof(int)))) {
		return -1;
	}
	for ( ; pdim < ivp->neqs; pdim++) ipos[pdim] = 0;
	
	for (pdim = 0; pdim < ivp->pint; pdim++) {
		ipos = memcpy(ipos+ivp->neqs, ipos, ivp->neqs*sizeof(int));
	}
	pdim = data->ys.iov_len / sizeof(double);
	if (!(ys = mpt_vecpar_alloc(&data->ys, neqs, sizeof(double)))) {
		return -1;
	}
	if (pdim < neqs) data->iopt[5] = 1;
	for ( ; pdim < neqs; pdim++) ys[pdim] = 0.0;
	
	/* reset counter on reinitialisation */
	for (pdim = 0; pdim < 6; pdim++) data->iopt[23+pdim] = 0;
	
	data->iopt[15] = 0;
	
	return neqs;
}
