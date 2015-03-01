/*!
 * wrapper to radau fortran routine.
 */

#include <errno.h>
#include <string.h>

#include "radau.h"

extern int mpt_radau_step(MPT_SOLVER_STRUCT(radau) *data, double *val, double tend)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *atol, *rtol;
	int neqs, tvec, imas, iout, idid, lrw, liw, *iwk;
	
	if (data->count.st.nfev < 0) {
		errno = EINVAL;
		return -1;
	}
	if (!data->fcn) {
		errno = EFAULT;
		return -1;
	}
	neqs = ivp->neqs * (ivp->pint + 1);
	
	if (ivp->neqs > 1 && data->rtol.base && data->atol.base) {
		tvec = 1; rtol = data->rtol.base; atol = data->atol.base;
	}
	else {
		tvec = 0; rtol = &data->rtol.d.val; atol = &data->atol.d.val;
	}
	if (!data->jac) data->ijac = 0;
	
	imas = data->mas ? 1 : 0;
	iout = data->sol ? 1 : 0;
	
	liw = data->iwork.iov_len / sizeof(int);
	lrw = data->rwork.iov_len / sizeof(double);
	iwk = data->iwork.iov_base;
	
	/* fortran routine call */
	radau_(&neqs, data->fcn, &ivp->last, val, &tend, &data->h, rtol, atol, &tvec,
	       data->jac, &data->ijac, &data->mljac, &data->mujac,
	       data->mas, &imas, &data->mlmas, &data->mumas,
	       data->sol, &iout, data->rwork.iov_base, &lrw, iwk, &liw,
	       data->rpar, data->ipar, &idid);
	
	for (tvec = 0; tvec < 6; tvec++) {
		data->count.raw[tvec] += iwk[tvec+13];
	}
	return idid < 0 ? idid : 0;
}
