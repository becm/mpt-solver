/*!
 * initialize limex instance.
 */

#include <stdlib.h>
#include <string.h>

#include "limex.h"

/*!
 * \ingroup mptSolverLimex
 * \brief clear LIMEX data
 * 
 * Clear resources used by LIMEX
 * 
 * \param data  LIMEX data
 */
extern void mpt_limex_fini(MPT_SOLVER_STRUCT(limex) *data)
{
	if (data->ivp.grid) {
		free(data->ivp.grid);
		data->ivp.grid = 0;
	}
	if (data->y) {
		free(data->y);
		data->y = 0;
	}
	if (data->ys) {
		free(data->ys);
		data->ys = 0;
	}
	if (data->ipos) {
		free(data->ipos);
		data->ipos = 0;
	}
	mpt_solver_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_solver_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	data->iopt[15] = -1;
}
/*!
 * \ingroup mptSolverLimex
 * \brief init LIMEX data
 * 
 * Initialize raw data for LIMEX use
 * 
 * \param data  LIMEX data
 */
extern void mpt_limex_init(MPT_SOLVER_STRUCT(limex) *data)
{
	const MPT_IVP_STRUCT(parameters) par = MPT_IVPPAR_INIT;
	
	data->ivp = par;
	
	data->t = 0.0;
	data->h = 0.0;
	
	data->y = 0;
	data->ys = 0;
	data->ipos = 0;
	data->ufcn = 0;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	
	(void) memset(&data->ropt,  0, sizeof(data->ropt));
	(void) memset(&data->iopt,  0, sizeof(data->iopt));
	
	(void) memset(&data->ifail, 0, sizeof(data->ifail));
	
	/* invalidate jacobian parameter */
	data->iopt[7] = data->iopt[8] = -1;
	
	data->iopt[15] = -1;  /* invalid solver state */
	
	data->fcn = 0;
	data->jac = 0;
}

