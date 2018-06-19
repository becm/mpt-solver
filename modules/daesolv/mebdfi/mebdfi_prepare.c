/*!
 * check/adapt memory sizes and integrator parameters.
 */

#include <stdlib.h>
#include <string.h>

#include "mebdfi.h"

extern int mpt_mebdfi_prepare(MPT_SOLVER_STRUCT(mebdfi) *me)
{
	int liw, lrw, *iwk;
	int neqs, pdim;
	double *v;
	size_t len;
	
	pdim = me->ivp.pint + 1;
	neqs = me->ivp.neqs * pdim;
	
	/* vector tolerances in sufficent dimension */
	if (neqs < 2 || (!me->rtol._base && !me->atol._base)) {
		pdim = 0;
	}
	if (mpt_solver_module_tol_check(&me->atol, me->ivp.neqs, pdim, __MPT_IVP_ATOL) < 0) {
		return -1;
	}
	if (mpt_solver_module_tol_check(&me->rtol, me->ivp.neqs, pdim, __MPT_IVP_RTOL) < 0) {
		return -1;
	}
	/* set banded matrix parameters */
	if (!me->jbnd) {
		me->mbnd[3] = neqs;
	}
	else {
		me->mbnd[2] = me->mbnd[0] + me->mbnd[1] + 1;
		me->mbnd[3] = 2*me->mbnd[0] + me->mbnd[1] + 1;
	}
	
	liw = neqs + 14;
	/* ignore wrong comments in mebdfi.f and set ACTUALLY needed size */
	lrw = (32 + 2 * me->mbnd[3]) * neqs + 2;
	
	if (!(iwk = mpt_solver_module_valloc(&me->iwork, liw, sizeof(int)))) {
		return -1;
	}
	if (!mpt_solver_module_valloc(&me->rwork, lrw, sizeof(double))) {
		return -1;
	}
	len = sizeof(*v) * neqs;
	if (!(v = me->y)) {
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		me->y = memset(v, 0, len);
	}
	if (!(v = me->yp)) {
		if (!(v = malloc(len))) {
			return MPT_ERROR(BadOperation);
		}
		me->yp = memset(v, 0, len);
	}
	
	me->state = 1; /* initial call for mebdfi() */
	
	/* reset counter on reinitialisation */
	for (liw = 0; liw < 7; liw++) iwk[4+liw] = 0;
	
	return neqs;
}
