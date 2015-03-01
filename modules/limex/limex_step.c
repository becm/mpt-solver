/*!
 * C wrapper to LIMEX fortran routine
 */

#include <errno.h>
#include <string.h>

#include "limex.h"

extern int mpt_limex_step(MPT_SOLVER_STRUCT(limex) *data, double *val, double tend)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *atol, *rtol;
	struct {
		int neqs, pint;
		const void *ufcn;
	} neq;
	
	if (data->iopt[15] < 0) {
		errno = EINVAL;
		return -1;
	}
	if (!data->fcn) {
		errno = EFAULT;
		return -1;
	}
	neq.neqs = ivp->neqs * (ivp->pint + 1);
	neq.pint = ivp->pint;
	neq.ufcn = data->ufcn;
	
	/* set tolerance flags and adresses */
	if (neq.neqs > 1 && (rtol = data->rtol.base) && (atol = data->atol.base)) {
		data->iopt[10] = 1;
	} else {
		data->iopt[10] = 0; rtol = &data->rtol.d.val; atol = &data->atol.d.val;
	}
	if (!data->jac) {
		data->iopt[6] = 0;
	}
	/* fortran routine call */
	limex_(&neq.neqs, data->fcn, data->jac, &ivp->last, &tend, val, data->ys.iov_base,
	       rtol, atol, &data->h, data->iopt, data->ropt, data->ipos.iov_base, data->ifail.raw);
	
	if (data->ifail.st.code < 0) {
		return data->ifail.st.code;
	}
	return 0;
}
