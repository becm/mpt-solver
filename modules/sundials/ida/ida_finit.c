/*!
 * initialize/clear IDA data.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ida/ida.h>

#include "sundials.h"

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
	if (data->sd.y) {
		N_VSetArrayPointer(0, data->sd.y);
		N_VDestroy(data->sd.y);
	}
	if (data->yp) {
		N_VDestroy(data->yp);
		data->yp = 0;
	}
	
	if (data->mem) IDAFree(&data->mem);
	
	MPT_IVPPAR_INIT(&data->ivp);
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	if (data->tmp.size && data->tmp.base) {
		free(data->tmp.base);
		data->tmp.base = 0;
		data->tmp.size = 0;
	}
}

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
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (data->sd.y) {
		N_VSetArrayPointer(0, data->sd.y);
		N_VDestroy(data->sd.y);
		data->sd.y = 0;
	}
	if (data->yp) {
		N_VDestroy(data->yp);
		data->yp = 0;
	}
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	if (data->tmp.size && data->tmp.base) {
		free(data->tmp.base);
		data->tmp.base = 0;
		data->tmp.size = 0;
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
	MPT_IVPPAR_INIT(&data->ivp);
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	data->ufcn = 0;
	
	data->tmp.base = 0;
	data->tmp.size = 0;
	
	data->yp = NULL;
	
	if (!(data->mem = IDACreate()))
		return IDA_MEM_NULL;
	
	IDASetUserData(data->mem, data);
	
	return 0;
}

