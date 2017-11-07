/*!
 * initialize/clear IDA data.
 */

#include <stdlib.h>
#include <string.h>

#include <ida/ida.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief reset IDA data
 * 
 * Prepare IDA data for new problem
 * 
 * \param data  IDA data
 */
extern void sundials_ida_reset(MPT_SOLVER_STRUCT(ida) *data)
{
	
	if (data->sd.y) {
		N_VDestroy(data->sd.y);
	}
	memset(&data->sd, 0, sizeof(data->sd));
	
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
}

/*!
 * \ingroup mptSundialsIda
 * \brief terminate IDA data
 * 
 * Clear resources used by IDA
 * 
 * \param data  IDA data
 */
extern void sundials_ida_fini(MPT_SOLVER_STRUCT(ida) *data)
{
	sundials_ida_reset(data);
	if (data->mem) {
		IDAFree(&data->mem);
		data->mem = NULL;
	}
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
extern int sundials_ida_init(MPT_SOLVER_STRUCT(ida) *data)
{
	const MPT_IVP_STRUCT(parameters) par = MPT_IVPPAR_INIT;
	
	if (!(data->mem = IDACreate())) {
		return IDA_MEM_NULL;
	}
	IDASetUserData(data->mem, data);
	
	data->ivp = par;
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	data->t = 0.0;
	data->hmax = 0.0;
	
	data->ufcn = 0;
	
	data->yp = NULL;
	
	data->tmp.base = 0;
	data->tmp.size = 0;
	
	return 0;
}

