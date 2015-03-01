/*!
 * initialize MEBDFI instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mebdfi.h"

extern void mpt_mebdfi_fini(MPT_SOLVER_STRUCT(mebdfi) *data)
{
	mpt_vecpar_alloc(&data->rwork, 0, 0);
	mpt_vecpar_alloc(&data->iwork, 0, 0);
	mpt_vecpar_alloc(&data->yp, 0, 0);
	
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (data->dmas) free(data->dmas);
	data->dmas = 0;
	
	data->state = -1;
}

extern int mpt_mebdfi_init(MPT_SOLVER_STRUCT(mebdfi) *data)
{
	/* allocate inital space for parameters */
	data->rwork.iov_base = data->iwork.iov_base = 0;
	if (!mpt_vecpar_alloc(&data->rwork, 64, sizeof(double))) {
		return -1;
	}
	if (!mpt_vecpar_alloc(&data->iwork, 16, sizeof(int))) {
		mpt_vecpar_alloc(&data->rwork, 0, 0); return -1;
	}
	/* initialize work data */
	memset(data->rwork.iov_base, 0, data->rwork.iov_len);
	memset(data->iwork.iov_base, 0, data->iwork.iov_len);
	
	MPT_IVPPAR_INIT(&data->ivp);
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	data->yp.iov_base = 0;
	data->yp.iov_len  = 0;
	
	data->h = 0.;
	
	data->jnum	= 0;	/* allow analytical jacobian */
	data->jbnd	= 0;	/* jacobian is banded */
	data->type	= 0;	/* normal step strategy */
	data->state	= -1;	/* invalid initial state */
	
	data->lout	= 6;	/* FORTRAN stdout */
	data->maxder	= 7;	/* default value */
	
	(void) memset(data->mbnd, 0,sizeof(data->mbnd));
	
	((int *) data->iwork.iov_base)[13] = 2000;
	
	data->rpar = 0;
	data->ipar = 0;
	
	data->fcn = 0;
	data->jac = 0;
	
	data->dmas = 0;
	
	return 0;
}

