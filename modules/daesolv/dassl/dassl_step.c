/*!
 * C wrapper to dDASSL fortran routine
 */

#include <errno.h>
#include <string.h>

#include "dassl.h"

extern int mpt_dassl_step(MPT_SOLVER_STRUCT(dassl) *data, double *val, double tend)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *atol, *rtol;
	int neqs, idid, lrw, liw;
	
	if (data->info[0] < 0) {
		errno = EINVAL;
		return -34;
	}
	
	if (!val || !data->fcn) {
		errno = EFAULT;
		return -35;
	}
	neqs = ivp->neqs * (ivp->pint + 1);
	
	/* set tolerance flags and adresses */
	if (neqs > 1 && (rtol = data->rtol.base) && (atol = data->atol.base)) {
		data->info[1] = 1;
	} else {
		data->info[1] = 0; rtol = &data->rtol.d.val; atol = &data->atol.d.val;
	}
	if (!data->jac) {
		data->info[4] = 0;
	}
	lrw = data->rwork.iov_len / sizeof(double);
	liw = data->iwork.iov_len / sizeof(int);
	
	/* fortran routine call */
	ddassl_(data->fcn, &neqs, &ivp->last, val, data->yp.iov_base, &tend, data->info,
	        rtol, atol, &idid, data->rwork.iov_base, &lrw, data->iwork.iov_base, &liw,
	        data->rpar, data->ipar, data->jac);
	
	return idid < 0 ? idid : 0;
}
