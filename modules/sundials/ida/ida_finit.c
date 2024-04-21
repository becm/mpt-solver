/*!
 * initialize/clear IDA data.
 */

#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <ida/ida.h>

#include "sundials.h"

static void resetValues(MPT_SOLVER_STRUCT(ida) *data)
{
	static const MPT_SOLVER_STRUCT(sundials_step) step = MPT_SOLVER_SUNDIALS_STEP_INIT;
	
	data->t = 0.0;
	
	data->step = step;
	data->mxstep = -1;
}

static void resetContext(MPT_SOLVER_STRUCT(ida) *data)
{
	if (data->yp) {
		N_VDestroy(data->yp);
		data->yp = 0;
	}
	mpt_solver_module_tol_check(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_solver_module_tol_check(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (data->tmp.size && data->tmp.base) {
		free(data->tmp.base);
		data->tmp.base = 0;
		data->tmp.size = 0;
	}
	mpt_sundials_fini(&data->sd);
}

/*!
 * \ingroup mptSundialsIda
 * \brief reset IDA data
 * 
 * Prepare IDA data for new problem
 * 
 * \param data  IDA data
 */
extern void mpt_sundials_ida_reset(MPT_SOLVER_STRUCT(ida) *data)
{
#if SUNDIALS_VERSION_MAJOR >= 6
	void *ctx = data->sd._sun_ctx_ref;
#endif
	resetValues(data);
	resetContext(data);
	mpt_sundials_init(&data->sd);
#if SUNDIALS_VERSION_MAJOR >= 6
	data->sd._sun_ctx_ref = ctx;
#endif
}

/*!
 * \ingroup mptSundialsIda
 * \brief terminate IDA data
 * 
 * Clear resources used by IDA
 * 
 * \param data  IDA data
 */
extern void mpt_sundials_ida_fini(MPT_SOLVER_STRUCT(ida) *data)
{
	mpt_solver_module_ivpset(&data->ivp, 0);
	resetContext(data);
}

/*!
 * \ingroup mptSundialsIda
 * \brief init IDA data
 * 
 * Initialize raw data for IDA use
 * 
 * \param data  IDA data
 * 
 * \return non-zero on failure
 */
extern int mpt_sundials_ida_init(MPT_SOLVER_STRUCT(ida) *data)
{
	const MPT_IVP_STRUCT(parameters) par = MPT_IVPPAR_INIT;
	
	data->mem = NULL;
	data->ivp = par;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	resetValues(data);
	
	data->ufcn = 0;
	
	data->yp = NULL;
	
	data->tmp.base = 0;
	data->tmp.size = 0;
	
	return 0;
}

