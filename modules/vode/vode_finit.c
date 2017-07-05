/*!
 * initialize/free vode instance.
 */

#include <stdlib.h>
#include <string.h>

#include "vode.h"

extern void mpt_vode_fini(MPT_SOLVER_STRUCT(vode) *data)
{
	if (data->ivp.grid) {
		free(data->ivp.grid);
		data->ivp.grid = 0;
	}
	if (data->y) {
		free(data->y);
		data->y = 0;
	}
	mpt_solver_valloc(&data->rwork, 0, 0);
	mpt_solver_valloc(&data->iwork, 0, 0);
	
	mpt_solver_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_solver_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	data->istate = -7;
}

extern void mpt_vode_init(MPT_SOLVER_STRUCT(vode) *data)
{
	const MPT_IVP_STRUCT(parameters) ivp = MPT_IVPPAR_INIT;
	const struct iovec vec = { 0, 0 };
	
	data->ivp = ivp;
	data->y = 0;
	data->t = 0;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	data->meth = 1;    /* default method (functional, Adams) */
	data->miter = 0;
	data->jsv = 1;
	
	data->istate = -7; /* invalid state before prepare() */
	
	data->itask = 1;   /* default to "normal" operation */
	data->iopt = 0;
	
	data->rwork = vec;
	data->iwork = vec;
	data->rpar = 0;
	data->ipar = 0;
	
	data->fcn = 0;
	data->jac = 0;
}

