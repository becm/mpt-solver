/*!
 * C wrapper to dDASSL fortran routine
 */

#include <string.h>

#include "dassl.h"

extern int mpt_dassl_step(MPT_SOLVER_STRUCT(dassl) *data, double tend)
{
	double *atol, *rtol;
	int neqs, idid, lrw, liw;
	
	if (data->info[0] < 0 || !data->fcn) {
		return MPT_ERROR(BadArgument);
	}
	neqs = data->ivp.neqs * (data->ivp.pint + 1);
	
	/* set tolerance flags and adresses */
	if (neqs > 1 && (rtol = data->rtol._base) && (atol = data->atol._base)) {
		data->info[1] = 1;
	} else {
		data->info[1] = 0;
		rtol = &data->rtol._d.val;
		atol = &data->atol._d.val;
	}
	if (!data->jac) {
		data->info[4] = 0;
	}
	lrw = data->rwork.iov_len / sizeof(double);
	liw = data->iwork.iov_len / sizeof(int);
	
	/* fortran routine call */
	ddassl_(data->fcn, &neqs, &data->t, data->y, data->yp, &tend, data->info,
	        rtol, atol, &idid, data->rwork.iov_base, &lrw, data->iwork.iov_base, &liw,
	        data->rpar, data->ipar, data->jac);
	
	return idid < 0 ? MPT_ERROR(BadOperation) : 0;
}
