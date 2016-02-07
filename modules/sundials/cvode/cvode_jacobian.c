/*!
 * wrapper for CVode Jacobian.
 */

#include <sundials/sundials_direct.h>
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
extern int sundials_cvode_jac_dense(long int n, realtype t,
                                    N_Vector y, N_Vector fy,
                                    DlsMat Jac, const MPT_SOLVER_STRUCT(cvode) *cv,
                                    N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	const MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	double *jac;
	int ld;
	
	(void) n;
	(void) fy;
	(void) tmp1; (void) tmp2; (void) tmp3;
	
	if (!cv || !(fcn = cv->ufcn) || !fcn->dae.jac) {
		return CV_MEM_NULL;
	}
	jac = DENSE_COL(Jac,0);
	ld = DENSE_COL(Jac,1) - jac;
	
	/* calculate jacobian */
	return fcn->dae.jac(fcn->dae.param, t, N_VGetArrayPointer(y), jac, ld);
}

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode banded jacobian wrapper
 * 
 * Wrapper to call mpt::ivpfcn jacobian from CVode solver.
 * For parameter description see Sundials documatation.
 * 
 * \return result of user jacobian function
 */
extern int sundials_cvode_jac_band(long int n, long int mu, long int ml,
                                   realtype t, N_Vector y, N_Vector fy,
                                   DlsMat Jac, const MPT_SOLVER_STRUCT(cvode) *cv,
                                   N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	const MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	double *jac;
	int ld;
	
	(void) n;
	(void) mu; (void) ml;
	(void) fy;
	(void) tmp1; (void) tmp2; (void) tmp3;
	
	if (!cv || !(fcn = cv->ufcn) || !fcn->dae.jac) {
		return CV_MEM_NULL;
	}
	/* BAND_COL(Jac,i) is diagonal element */
	jac = BAND_COL(Jac,0);
	ld  = BAND_COL(Jac,1) - jac - 1;
	
	/* calculate jacobian */
	return fcn->dae.jac(fcn->dae.param, t, N_VGetArrayPointer(y), jac, ld);
}

