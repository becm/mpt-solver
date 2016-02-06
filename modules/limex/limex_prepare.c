/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <string.h>
#include <stdlib.h>

#include "limex.h"

extern int mpt_limex_prepare(MPT_SOLVER_STRUCT(limex) *data)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	int neqs, pdim;
	double *v;
	int *ipos;
	
	ivp  = &data->ivp;
	pdim = ivp->pint + 1;
	neqs = ivp->neqs * pdim;
	
	if (data->iopt[7] < 0) data->iopt[7] = ivp->neqs;
	if (data->iopt[8] < 0) data->iopt[8] = ivp->neqs;
	
	/* vector tolerances in sufficent dimension */
	if (ivp->neqs < 2 || (!data->rtol.base && !data->atol.base)) {
		pdim = 0;
	}
	if (mpt_vecpar_cktol(&data->atol, neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (mpt_vecpar_cktol(&data->rtol, neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return MPT_ERROR(BadOperation);
	}
	if (!(ipos = data->ipos)) {
		size_t len = sizeof(*ipos) * neqs;
		if (!(ipos = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->ipos = memset(ipos, 0, len);
	}
	if (!(v = data->y)) {
		size_t len = sizeof(*v) * neqs;
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->y = memset(v, 0, len);
	}
	if (!(v = data->ys)) {
		size_t len = sizeof(*v) * neqs;
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->ys = memset(v, 0, len);
		
		/* determine consistent initstate */
		data->iopt[5] = 1;
	}
	/* reset counter on reinitialisation */
	for (pdim = 0; pdim < 6; pdim++) data->iopt[23+pdim] = 0;
	
	data->iopt[15] = 0;
	
	return neqs;
}
