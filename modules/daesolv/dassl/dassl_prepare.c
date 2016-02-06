/*!
 * check/adapt dDASSL memory sizes and integrator parameters
 */

#include <stdlib.h>
#include <string.h>

#include "dassl.h"

extern int mpt_dassl_prepare(MPT_SOLVER_STRUCT(dassl) *data)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	double *v;
	int *iwork, lrw, liw;
	int neqs, pdim;
	size_t len;
	
	ivp  = &data->ivp;
	pdim = ivp->pint + 1;
	neqs = ivp->neqs * pdim;
	
	/* vector tolerances in sufficent dimension */
	if (ivp->neqs < 2 || (!data->rtol.base && !data->atol.base)) {
		pdim = 0;
	}
	if (mpt_vecpar_cktol(&data->atol, ivp->neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	if (mpt_vecpar_cktol(&data->rtol, ivp->neqs, pdim, __MPT_IVP_RTOL) < 0) {
		return -1;
	}
	iwork = data->iwork.iov_base;
	
	liw	= 20 + neqs;
	lrw	= 40 + ((data->info[8] ? iwork[2] : 5) + 4) * neqs;
	
	/* full jacobian */
	if ( !data->info[5] )
		lrw += neqs * neqs;
	/* banded jacobian */
	else {
		lrw += (2*iwork[0]+iwork[1]+1) * neqs;
		/* banded numeric jacobian */
		if ( !data->jac )
			lrw += 2 * (neqs/(iwork[0]+iwork[1]+1) + 1);
	}
	
	if (!(iwork = mpt_vecpar_alloc(&data->iwork, liw, sizeof(int)))) {
		return -1;
	}
	if (!mpt_vecpar_alloc(&data->rwork, lrw, sizeof(double))) {
		return -1;
	}
	len = neqs * sizeof(double);
	if (!(v = data->y)) {
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->y = memset(v, 0, len);
	}
	if (!(v = data->yp)) {
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		data->yp = memset(v, 0, len);
	}
	data->info[0] = 0;
	
	/* reset counter on reinitialisation */
	for (liw = 0; liw < 5; liw++) {
		iwork[10+liw] = 0;
	}
	return neqs;
}
