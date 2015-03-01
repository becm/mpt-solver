/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <stdlib.h>
#include <errno.h>

#include "vode.h"

extern int mpt_vode_prepare(MPT_SOLVER_STRUCT(vode) *data, int neqs, int pint)
{
	int mf, mu, ml;     /* method flag, upper/lower band */
	int liw, lrw, *iwk; /* length of work arrays */
	
	if (neqs < 1 || pint++ < 0) {
		errno = EINVAL; return -2;
	}
	mf = (data->atol.base && neqs > 1) ? pint : 0;
	if (mpt_vecpar_cktol(&data->atol, neqs, mf, __MPT_IVP_ATOL) < 0)
		return -1;
		
	mf = (data->rtol.base && neqs > 1) ? pint : 0;
	if (mpt_vecpar_cktol(&data->rtol, neqs, mf, __MPT_IVP_RTOL) < 0)
		return -1;
	
	neqs *= pint;	/* total dimension for solver */
	
	switch ((mf = data->jsv * (data->meth * 10 + data->miter))) {
		case  10: case 13: case 20: case 23:
			liw = 30; break;
		default:
			liw = 30 + neqs;
	}
	iwk = data->iwork.iov_base;
	ml  = iwk[0];
	mu  = iwk[1];
	
	switch ( mf ) {
		case  10:
		case -10: lrw = -2 + 16 * neqs; break;
		case  11:
		case  12: lrw = 16 * neqs + 2 * neqs * neqs; break;
		case -11:
		case -12: lrw = 16 * neqs + neqs * neqs; break;
		case  13:
		case -13: lrw = 17 * neqs; break;
		case  14:
		case  15: lrw = 18 * neqs + (3 * ml + 2 * mu) * neqs; break;
		case -14:
		case -15: lrw = 17 * neqs + (2 * ml + mu) * neqs; break;
		case  20:
		case -20: lrw = -2 + 9 * neqs; break;
		case  21:
		case  22: lrw = 9 * neqs + 2 * neqs * neqs; break;
		case -21:
		case -22: lrw = 9 * neqs + neqs * neqs; break;
		case  23:
		case -23: lrw = 10 * neqs; break;
		case  24:
		case  25: lrw = 11 * neqs + (3 * ml + 2 * mu) * neqs; break;
		case -24:
		case -25: lrw = 10 * neqs + (2 * ml + mu) * neqs; break;
		default:
			errno = ENOSYS; return -1;
	}
	
	lrw += 22;
	
	if (!(iwk = mpt_vecpar_alloc(&data->iwork, liw, sizeof(int))))
		return -1;
	if (!mpt_vecpar_alloc(&data->rwork, lrw, sizeof(double)))
		return -1;
	
	data->istate = 1;
	
	/* reset counter on reinitialisation */
	for (mf = 0; mf < 3; mf++)
		iwk[10+mf] = 0;
	for (mf = 0; mf < 4; mf++)
		iwk[18+mf] = 0;
	
	return neqs;
}
