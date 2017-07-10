/*!
 * initialize/clear dDASSL solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dassl.h"

extern void mpt_dassl_fini(MPT_SOLVER_STRUCT(dassl) *data)
{
	mpt_solver_valloc(&data->rwork, 0, 0);
	mpt_solver_valloc(&data->iwork, 0, 0);
	
	if (data->y) {
		free(data->y);
		data->y = 0;
	}
	if (data->yp) {
		free(data->yp);
		data->yp = 0;
	}
	if (data->dmas) {
		free(data->dmas);
		data->dmas = 0;
	}
	mpt_solver_tol_check(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_solver_tol_check(&data->atol, 0, 0, __MPT_IVP_ATOL);
}

extern void mpt_dassl_init(MPT_SOLVER_STRUCT(dassl) *data)
{
	const MPT_IVP_STRUCT(parameters) par = MPT_IVPPAR_INIT;
	
	data->ivp = par;
	data->t = 0.0;
	
	/* delay work vector initialization */
	data->rwork.iov_base = 0; data->rwork.iov_len  = 0;
	data->iwork.iov_base = 0; data->iwork.iov_len  = 0;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	data->y  = 0;
	data->yp = 0;
	data->dmas = 0;
	
	data->rpar = 0;
	data->ipar = 0;
	
	data->fcn = 0;
	data->jac = 0;
	
	/* default: not prepared, inconsistent inital values */
	(void) memset(data->info, 0, sizeof(data->info));
	data->info[0] = -1;
	data->info[10] = 1;
}

