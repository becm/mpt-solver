/*!
 * prepare IDA solver
 */

#include <stdio.h>

#include <ida/ida.h>

#define _SUNDIALS_GENERIC_TYPE(x) void
#include "sundials.h"

#include "module_functions.h"

/*!
 * \ingroup mptSundialsIda
 * \brief prepare IDA solver
 * 
 * Prepare IDA solver data for step operation.
 * 
 * \param data  IDA solver data
 * \param val   initial values
 * 
 * \return non-zero on error
 */
extern int mpt_sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *ida)
{
	void *ida_mem;
	long neqs;
	int err;
	
	if (!ida) {
		return MPT_ERROR(BadArgument);
	}
	if (!(ida_mem = ida->mem)) {
#if SUNDIALS_VERSION_MAJOR >= 6
		if (!(ida_mem = IDACreate(mpt_sundials_context(&ida->sd)))) {
#else
		if (!(ida_mem = IDACreate())) {
#endif
			return MPT_ERROR(BadOperation);
		}
		if (IDASetUserData(ida_mem, ida) != IDA_SUCCESS) {
			IDAFree(&ida_mem);
			return MPT_ERROR(BadOperation);
		}
		ida->mem = ida_mem;
	}
	
	if ((neqs = ida->ivp.neqs) < 1 || (neqs *= (ida->ivp.pint + 1)) <= 0) {
		return MPT_ERROR(BadArgument);
	}
	
	/* prepare initial vector */
	if (!ida->sd.y) {
#if SUNDIALS_VERSION_MAJOR >= 6
		if (!(ida->sd.y = mpt_sundials_nvector(neqs, mpt_sundials_context(&ida->sd)))) {
#else
		if (!(ida->sd.y = mpt_sundials_nvector(neqs))) {
#endif
			return MPT_ERROR(BadOperation);
		}
		N_VConst(0, ida->sd.y);
	}
	if (!ida->yp) {
		if (!(ida->yp = N_VClone(ida->sd.y))) {
			return MPT_ERROR(BadOperation);
		}
		N_VConst(0, ida->yp);
	}
	err = IDAInit(ida_mem, mpt_sundials_ida_fcn, ida->t, ida->sd.y, ida->yp);
	
	/* prepare tolerances */
	if (!ida->atol._base && !ida->rtol._base) {
		err = IDASStolerances(ida_mem, ida->rtol._d.val, ida->atol._d.val);
	} else {
		if (mpt_solver_module_tol_check(&ida->rtol, ida->ivp.neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_solver_module_tol_check(&ida->atol, ida->ivp.neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = IDAWFtolerances(ida_mem, mpt_sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	if (!ida->sd.linsol) {
		ida->sd.linsol = MPT_SOLVER_SUNDIALS(Direct);
	}
	if (!ida->sd.jacobian) {
		if (!ida->ivp.pint || ida->sd.ml >= neqs || ida->sd.mu >= neqs) {
			ida->sd.jacobian = SUNDIALS_DENSE;
		} else {
			ida->sd.jacobian = SUNDIALS_BAND;
		}
	}
	if (ida->sd.mu < 0) {
		ida->sd.mu = ida->ivp.pint ? ida->ivp.neqs : ida->ivp.neqs - 1;
	}
	if (ida->sd.ml < 0) {
		ida->sd.ml = ida->ivp.pint ? ida->ivp.neqs : ida->ivp.neqs - 1;
	}
	if ((err = mpt_sundials_linear(&ida->sd, neqs)) < 0) {
		return err;
	}
	if ((err = IDASetLinearSolver(ida_mem, ida->sd.LS, ida->sd.A)) < 0) {
		return err;
	}
	if (ida->sd.A) {
		if (!ida->ufcn || !ida->ufcn->jac.fcn) {
			ida->sd.linsol |= MPT_SOLVER_SUNDIALS(Numeric);
		}
		if (!(ida->sd.linsol & MPT_SOLVER_SUNDIALS(Numeric))) {
			IDASetJacFn(ida_mem, mpt_sundials_ida_jac);
		}
	}
	return err;
}

