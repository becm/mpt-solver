/*!
 * initialize/finalize CVode instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "cvode/cvode_impl.h"
#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief terminate CVode data
 * 
 * Clear resources used by CVode
 * 
 * \param data  CVode data
 */
extern void sundials_cvode_fini(MPT_SOLVER_STRUCT(cvode) *data)
{
	MPT_IVPPAR_INIT(&data->ivp);
	
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (data->sd.y) {
		N_VDestroy(data->sd.y);
	}
	if (data->mem) CVodeFree(&data->mem);
	memset(&data->sd, 0, sizeof(data->sd));
}

/*!
 * \ingroup mptSundialsCVode
 * \brief reset CVode data
 * 
 * Prepare CVode data for new problem
 * 
 * \param data  CVode data
 */
extern void sundials_cvode_reset(MPT_SOLVER_STRUCT(cvode) *data)
{
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (data->sd.y) {
		N_VDestroy(data->sd.y);
		data->sd.y = 0;
	}
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
}

/*!
 * \ingroup mptSundialsCVode
 * \brief init CVode data
 * 
 * Initialize raw data for CVode use
 * 
 * \param data  CVode data
 * 
 * \return non-zero on failure
 */
extern int sundials_cvode_init(MPT_SOLVER_STRUCT(cvode) *data)
{
	MPT_IVPPAR_INIT(&data->ivp);
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	if (!(data->mem = CVodeCreate(CV_ADAMS, CV_NEWTON))) {
		return CV_MEM_NULL;
	}
	return 0;
}
