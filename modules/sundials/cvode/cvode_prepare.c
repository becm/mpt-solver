/*!
 * prepare CVode solver descriptor for step operation
 */

#include <string.h>

#include <stdio.h>
#include <cvode/cvode_diag.h>

#include <cvode/cvode_direct.h>

#include <cvode/cvode_impl.h>

#define _SUNDIALS_GENERIC_TYPE(x) void
#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief prepare CVode solver
 * 
 * Prepare CVode solver data for step operation.
 * 
 * \param data  CVode solver data
 * \param val   initial values
 * 
 * \return non-zero on error
 */
extern int sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *cv)
{
	CVodeMem cv_mem;
	long neqs;
	int err;
	
	if (!cv || !(cv_mem = cv->mem)) {
		return MPT_ERROR(BadArgument);
	}
	if ((neqs = cv->ivp.neqs) < 1 || (neqs *= (cv->ivp.pint + 1)) <= 0) {
		return MPT_ERROR(BadArgument);
	}
	
	/* prepare initial vector */
	if (!cv->sd.y) {
		realtype *y;
		if (!(cv->sd.y = sundials_nvector_new(neqs))) {
			return MPT_ERROR(BadOperation);
		}
		y = N_VGetArrayPointer(cv->sd.y);
		memset(y, 0, neqs * sizeof(*y));
	}
	if ((err = CVodeInit(cv_mem, sundials_cvode_fcn, cv->t, cv->sd.y)) < 0) {
		return err;
	}
	/* prepare tolerances */
	if (!cv->atol.base && !cv->rtol.base) {
		err = CVodeSStolerances(cv_mem, cv->rtol.d.val, cv->atol.d.val);
	} else {
		if (mpt_solver_module_tol_check(&cv->rtol, cv->ivp.neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_solver_module_tol_check(&cv->atol, cv->ivp.neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = CVodeWFtolerances(cv_mem, sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	if (!cv->sd.jacobian) {
		cv->sd.linalg = 0;
	}
	else if (!cv->sd.linalg) {
		cv->sd.linalg = MPT_SOLVER_ENUM(SundialsDls);
		err = cv->sd.jacobian & MPT_SOLVER_ENUM(SundialsJacType);
		if (err == MPT_SOLVER_ENUM(SundialsJacBand)) {
			if (cv->sd.mu < 0) {
				cv->sd.mu = cv->ivp.neqs;
			}
			if (cv->sd.ml < 0) {
				cv->sd.ml = cv->ivp.neqs;
			}
		}
	}
	if (!cv->sd.linalg) {
		return CVodeSetIterType(cv_mem, CV_FUNCTIONAL);
	}
	if ((err = sundials_linear(&cv->sd, neqs)) < 0) {
		return err;
	}
	if ((err = CVDlsSetLinearSolver(cv_mem, cv->sd.LS, cv->sd.A)) < 0) {
		return err;
	}
	if (cv->sd.A
	    && !(cv->sd.jacobian & MPT_SOLVER_ENUM(SundialsJacNumeric))
	    && cv->ufcn
	    && cv->ufcn->jac.fcn) {
		CVDlsSetJacFn(cv_mem, sundials_cvode_jac);
	}
	return err;
}
