/*!
 * initialize/free vode instance.
 */

#include <stdlib.h>
#include <string.h>

#include "vode.h"

extern void mpt_vode_fini(MPT_SOLVER_STRUCT(vode) *data)
{
	mpt_vecpar_alloc(&data->rwork, 0, 0);
	mpt_vecpar_alloc(&data->iwork, 0, 0);
	
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	data->istate = -7;
}

extern void mpt_vode_init(MPT_SOLVER_STRUCT(vode) *data)
{
	/* allocate inital space for parameters */
	data->rwork.iov_base = 0; data->rwork.iov_len = 0;
	data->iwork.iov_base = 0; data->iwork.iov_len = 0;
	
	MPT_IVPPAR_INIT(&data->ivp);
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	data->meth = 1;    /* default method (functional, Adams) */
	data->miter = 0;
	data->jsv = 1;
	
	data->istate = -7; /* invalid state before prepare() */
	
	data->itask = 1;   /* default to "normal" operation */
	data->iopt = 0;
	
	data->rpar = 0;
	data->ipar = 0;
	
	data->fcn = 0;
	data->jac = 0;
}

