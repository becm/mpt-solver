/*!
 * prepare CVode solver descriptor for step operation
 */

#include <string.h>

#include <stdio.h>

#include <sundials/sundials_nonlinearsolver.h>
#include <sunnonlinsol/sunnonlinsol_fixedpoint.h>

#include <cvode/cvode_diag.h>
#include <cvode/cvode_ls.h>

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
extern int mpt_sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *cv)
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
		if (!(cv->sd.y = mpt_sundials_nvector(neqs))) {
			return MPT_ERROR(BadOperation);
		}
		y = N_VGetArrayPointer(cv->sd.y);
		memset(y, 0, neqs * sizeof(*y));
	}
	if ((err = CVodeInit(cv_mem, mpt_sundials_cvode_fcn, cv->t, cv->sd.y)) < 0) {
		return err;
	}
	/* prepare tolerances */
	if (!cv->atol._base && !cv->rtol._base) {
		err = CVodeSStolerances(cv_mem, cv->rtol._d.val, cv->atol._d.val);
	} else {
		if (mpt_solver_module_tol_check(&cv->rtol, cv->ivp.neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_solver_module_tol_check(&cv->atol, cv->ivp.neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = CVodeWFtolerances(cv_mem, mpt_sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	/* CVodeInit() assigns Newton (rootfind) nonlinear solver by default */
	if (!cv->sd.stype) {
		/* TODO: variable acc. vector count */
		SUNNonlinearSolver NLS = SUNNonlinSol_FixedPoint(cv->sd.y, 0);
		if (!NLS) {
			return MPT_ERROR(BadOperation);
		}
		err = CVodeSetNonlinearSolver(cv_mem, NLS);
		if (err < 0) {
			SUNNonlinSolFree(NLS);
			return err;
		}
		/* WARNING: CVodeSetNonlinearSolver() ALWAYS calls SUNNonlinSolFree() for previous solver.
		 * Make CVODE own nonlinear solver reference for consistency.
		 */
		cv_mem->ownNLS = SUNTRUE;
		return err;
	}
	if (cv->sd.stype & MPT_SOLVER_SUNDIALS(Direct)) {
		if (!cv->sd.jacobian) {
			return CVDiag(cv_mem);
		}
	}
	if (cv->sd.mu < 0) {
		cv->sd.mu = cv->ivp.pint ? cv->ivp.neqs : cv->ivp.neqs - 1;
	}
	if (cv->sd.ml < 0) {
		cv->sd.ml = cv->ivp.pint ? cv->ivp.neqs : cv->ivp.neqs - 1;
	}
	if ((err = mpt_sundials_linear(&cv->sd, neqs)) < 0) {
		return err;
	}
	if ((err = CVodeSetLinearSolver(cv_mem, cv->sd.LS, cv->sd.A)) < 0) {
		return err;
	}
	if (cv->sd.A) {
		if (!cv->ufcn || !cv->ufcn->jac.fcn) {
			cv->sd.stype |= MPT_SOLVER_SUNDIALS(Numeric);
		}
		if (!(cv->sd.stype & MPT_SOLVER_SUNDIALS(Numeric))) {
			CVodeSetJacFn(cv_mem, mpt_sundials_cvode_jac);
		}
	}
	return err;
}
