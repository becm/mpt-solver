/*!
 * initialize radau instance.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "radau.h"

extern void mpt_radau_fini(MPT_SOLVER_STRUCT(radau) *data)
{
	if (data->dmas) {
		free(data->dmas);
		data->dmas = 0;
	}
	if (data->y) {
		free(data->y);
		data->y = 0;
	}
	mpt_vecpar_alloc(&data->rwork, 0, 0);
	mpt_vecpar_alloc(&data->iwork, 0, 0);
	
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
}

extern void mpt_radau_init(MPT_SOLVER_STRUCT(radau) *data)
{
	
	MPT_IVPPAR_INIT(&data->ivp);
	
	(void) memset(&data->count, 0, sizeof(data->count));
	/* indicate unprepared state */
	data->count.st.nfev = -1;
	data->ijac = 0;
	
	data->t = 0.;
	data->h = 0.;
	
	data->y = 0;
	data->dmas = 0;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	/* allocate inital space for parameters */
	data->rwork.iov_base = 0; data->rwork.iov_len =  0;
	data->iwork.iov_base = 0; data->iwork.iov_len =  0;
	
	data->rpar = 0;
	data->ipar = 0;
	
	/* invalidate band parameter */
	data->mljac = data->mujac = data->mlmas = data->mumas = -1;
	
	data->fcn = 0;
	data->jac = 0;
	data->mas = 0;
	data->sol = 0;
}

