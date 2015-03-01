/*!
 * check/adapt dDASSL memory sizes and integrator parameters
 */

#include <stdlib.h>
#include <errno.h>

#include "dassl.h"

extern int mpt_dassl_prepare(MPT_SOLVER_STRUCT(dassl) *data, int neqs, int pdim)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	double *yp;
	int *iwork, lrw, liw;
	
	if (neqs < 1 || pdim < 0) {
		errno = EINVAL;
		return -2;
	}
	
	ivp  = &data->ivp;
	pdim = (ivp->pint = pdim) + 1;
	neqs = (ivp->neqs = neqs) * pdim;
	
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
	pdim = data->yp.iov_len / sizeof(*yp);
	
	if (!(yp = mpt_vecpar_alloc(&data->yp, neqs, sizeof(*yp)))) {
		return -1;
	}
	for ( ; pdim < neqs; pdim++) {
		yp[pdim] = 0.0;
	}
	data->info[0] = 0;
	
	/* reset counter on reinitialisation */
	for (liw = 0; liw < 5; liw++) {
		iwork[10+liw] = 0;
	}
	return neqs;
}
