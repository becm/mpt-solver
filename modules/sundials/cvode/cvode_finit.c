/*!
 * initialize/finalize CVode instance
 */

#include <stdlib.h>
#include <string.h>

#include "cvode/cvode_impl.h"
#include "sundials.h"

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
	
	if (data->mem) {
		CVodeFree(&data->mem);
		data->mem = NULL;
	}
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
	if (!(data->mem = CVodeCreate(CV_ADAMS))) {
		return CV_MEM_NULL;
	}
	data->ivp = par;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	memset(&data->sd, 0, sizeof(data->sd));
	
	data->t = 0.0;
	data->hmax = 0.0;
	
	data->ufcn = NULL;
	
	return 0;
}
