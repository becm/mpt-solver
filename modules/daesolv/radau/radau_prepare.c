/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "radau.h"

extern int mpt_radau_prepare(MPT_SOLVER_STRUCT(radau) *data, int neqs, int pdim)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	int nsmax, liw, lrw, *iwork;
	
	if (neqs < 1 || pdim < 0) {
		errno = EINVAL;
		return -2;
	}
	
	if (data->mljac < 0) data->mljac = neqs;
	if (data->mujac < 0) data->mujac = neqs;
	if (data->mlmas < 0) data->mlmas = data->mljac;
	if (data->mumas < 0) data->mumas = data->mujac;
	
	iwork = data->iwork.iov_base;
	
	pdim  = (ivp->pint = pdim) + 1;
	neqs  = (ivp->neqs = neqs) * pdim;
	nsmax = (iwork && iwork[11]) ? iwork[11] : 7;
	
	/* vector tolerances in sufficent dimension */
	if (neqs < 2 || (!data->rtol.base && !data->atol.base)) {
		pdim = 0;
	}
	if (mpt_vecpar_cktol(&data->rtol, ivp->neqs, pdim, __MPT_IVP_RTOL) < 0) {
		return -1;
	}
	if (mpt_vecpar_cktol(&data->atol, ivp->neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	/* (2 + (NSMAX - 1) / 2) * N + 20 */
	liw = (2 + (nsmax-1) / 2) * neqs + 20;
	/* LJAC */	lrw  = (data->mljac >= neqs) ? neqs : data->mljac+data->mujac+1;
	/* LMAS */	lrw += data->mas ? ((data->mljac >= neqs) ? neqs : data->mlmas+data->mumas+1) : 0;
	/* NSMAX*LE */	lrw += nsmax * ((data->mljac >= neqs) ? neqs : 2*data->mljac+data->mujac+1);
	/* N * (LJAC + LMAS + NSMAX*LE + 3*NSMAX + 3) + 20 */
	lrw = neqs * (lrw + 3*nsmax + 3) + 20;
	
	if (!mpt_vecpar_alloc(&data->iwork, liw, sizeof(int))) {
		return -1;
	}
	if (!mpt_vecpar_alloc(&data->rwork, lrw, sizeof(double))) {
		return -1;
	}
	(void) memset(&data->count, 0, sizeof(data->count));
	
	return neqs;
}
