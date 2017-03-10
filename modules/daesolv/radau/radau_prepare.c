/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <string.h>

#include "radau.h"

extern int mpt_radau_prepare(MPT_SOLVER_STRUCT(radau) *rd)
{
	int nsmax, liw, lrw, *iwork;
	int neqs, pdim;
	
	if (rd->mljac < 0) rd->mljac = rd->ivp.neqs;
	if (rd->mujac < 0) rd->mujac = rd->ivp.neqs;
	if (rd->mlmas < 0) rd->mlmas = rd->mljac;
	if (rd->mumas < 0) rd->mumas = rd->mujac;
	
	iwork = rd->iwork.iov_base;
	
	pdim  = rd->ivp.pint + 1;
	neqs  = rd->ivp.neqs * pdim;
	nsmax = (iwork && iwork[11]) ? iwork[11] : 7;
	
	/* vector tolerances in sufficent dimension */
	if (neqs < 2 || (!rd->rtol.base && !rd->atol.base)) {
		pdim = 0;
	}
	if (mpt_vecpar_cktol(&rd->rtol, rd->ivp.neqs, pdim, __MPT_IVP_RTOL) < 0) {
		return -1;
	}
	if (mpt_vecpar_cktol(&rd->atol, rd->ivp.neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	/* (2 + (NSMAX - 1) / 2) * N + 20 */
	liw = (2 + (nsmax-1) / 2) * neqs + 20;
	/* LJAC */	lrw  = (rd->mljac >= neqs) ? neqs : rd->mljac+rd->mujac+1;
	/* LMAS */	lrw += rd->mas ? ((rd->mljac >= neqs) ? neqs : rd->mlmas+rd->mumas+1) : 0;
	/* NSMAX*LE */	lrw += nsmax * ((rd->mljac >= neqs) ? neqs : 2*rd->mljac+rd->mujac+1);
	/* N * (LJAC + LMAS + NSMAX*LE + 3*NSMAX + 3) + 20 */
	lrw = neqs * (lrw + 3*nsmax + 3) + 20;
	
	if (!mpt_vecpar_alloc(&rd->iwork, liw, sizeof(int))) {
		return -1;
	}
	if (!mpt_vecpar_alloc(&rd->rwork, lrw, sizeof(double))) {
		return -1;
	}
	(void) memset(&rd->count, 0, sizeof(rd->count));
	
	return neqs;
}
