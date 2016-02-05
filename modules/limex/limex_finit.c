/*!
 * initialize limex instance.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

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
	mpt_vecpar_cktol(&data->rtol, 0, 0, __MPT_IVP_RTOL);
	mpt_vecpar_cktol(&data->atol, 0, 0, __MPT_IVP_ATOL);
	
	data->iopt[15] = -1;
}
/*!
 * \ingroup mptSolverLimex
 * \brief reset LIMEX data
 * 
 * Prepare LIMEX data for new problem
 * 
 * \param data  IDA data
 */
extern void mpt_limex_reset(MPT_SOLVER_STRUCT(limex) *data)
{
	mpt_limex_fini(data);
	data->h = 0.;
	
	(void) memset(&data->ifail, 0, sizeof(data->ifail));
	(void) memset(&data->ropt,  0, sizeof(data->ropt));
	(void) memset(&data->iopt,  0, sizeof(data->iopt));
	
	/* invalidate jacobian parameter */
	data->iopt[7] = data->iopt[8] = -1;
	
	/* invalidate solver state */
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
	MPT_IVPPAR_INIT(&data->ivp);
	
	data->t = 0;
	
	data->y = 0;
	data->ys = 0;
	data->ipos = 0;
	data->ufcn = 0;
	
	MPT_VECPAR_INIT(&data->rtol, __MPT_IVP_RTOL);
	MPT_VECPAR_INIT(&data->atol, __MPT_IVP_ATOL);
	
	data->h = 0.;
	
	(void) memset(&data->ropt,  0, sizeof(data->ropt));
	(void) memset(&data->iopt,  0, sizeof(data->iopt));
	
	(void) memset(&data->ifail, 0, sizeof(data->ifail));
	
	/* invalidate jacobian parameter */
	data->iopt[7] = data->iopt[8] = -1;
	
	data->iopt[15] = -1;  /* invalid solver state */
	
	data->fcn = 0;
	data->jac = 0;
}

