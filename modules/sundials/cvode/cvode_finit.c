/*!
 * initialize/finalize CVode instance
 */

#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "cvode/cvode.h"

#include "sundials.h"

static void resetValues(MPT_SOLVER_STRUCT(cvode) *data) {
	static const MPT_SOLVER_STRUCT(sundials_step) step = MPT_SOLVER_SUNDIALS_STEP_INIT;
	
	data->t = 0.0;
	
	data->step = step;
	data->mxstep = -1;
	data->mxhnil = -1;
	
	data->method = CV_ADAMS;
	data->maxord = 0;
}

/*!
 * \ingroup mptSundialsCVode
 * \brief reset CVode data
 * 
 * Prepare CVode data for new problem
 * 
 * \param data  CVode data
 */
extern void mpt_sundials_cvode_reset(MPT_SOLVER_STRUCT(cvode) *data)
{
	mpt_sundials_fini(&data->sd);
	
	mpt_solver_module_tol_check(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_solver_module_tol_check(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	if (data->mem) {
		CVodeFree(&data->mem);
		data->mem = NULL;
	}
	resetValues(data);
}

/*!
 * \ingroup mptSundialsCVode
 * \brief terminate CVode data
 * 
 * Clear resources used by CVode
 * 
 * \param data  CVode data
 */
extern void mpt_sundials_cvode_fini(MPT_SOLVER_STRUCT(cvode) *data)
{
	mpt_solver_module_ivpset(&data->ivp, 0);
	
	mpt_sundials_cvode_reset(data);
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
extern int mpt_sundials_cvode_init(MPT_SOLVER_STRUCT(cvode) *data)
{
	const MPT_IVP_STRUCT(parameters) par = MPT_IVPPAR_INIT;
	
	data->mem = NULL;
	data->ivp = par;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	resetValues(data);
	
	data->ufcn = NULL;
	
	return 0;
}
