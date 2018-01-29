/*!
 * wrapper for CVode Jacobian.
 */

#include <stdio.h>
#include <sundials/sundials_direct.h>

#include <sundials/sundials_nvector.h>

#include <sundials/sundials_matrix.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunmatrix/sunmatrix_band.h>

#include <cvode/cvode.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode dense jacobian wrapper
 * 
 * Wrapper to call mpt::ivpfcn jacobian from CVode solver.
 * For parameter description see Sundials documatation.
 * 
 * \return result of user jacobian function
 */
extern int mpt_sundials_cvode_jac(realtype t,
                                  N_Vector y, N_Vector fy,
                                  SUNMatrix Jac, const MPT_SOLVER_STRUCT(cvode) *cv,
                                  N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	const MPT_IVP_STRUCT(odefcn) *fcn;
	double *jac;
	sunindextype ld;
	
	if (!cv || !(fcn = cv->ufcn) || !fcn->jac.fcn) {
		return CV_MEM_NULL;
	}
	(void) fy;
	(void) tmp1; (void) tmp2; (void) tmp3;
	
	ld = SUNMatGetID(Jac);
	
	if (ld == SUNMATRIX_DENSE) {
		jac = SM_DATA_D(Jac);
		ld  = SM_ROWS_D(Jac);
	}
	else if (ld == SUNMATRIX_BAND) {
		jac = SM_DATA_B(Jac);
		ld  = SM_LDIM_B(Jac);
	}
	else {
		return MPT_ERROR(BadArgument);
	}
	/* calculate jacobian */
	return fcn->jac.fcn(fcn->jac.par, t, N_VGetArrayPointer(y), jac, ld);
}
