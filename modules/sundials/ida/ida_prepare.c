/*!
 * prepare IDA solver
 */

#include <stdio.h>
#include <ida/ida_direct.h>

#include <ida/ida_impl.h>

#define _SUNDIALS_GENERIC_TYPE(x) void
#include "sundials.h"

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
extern int sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *ida)
{
	IDAMem ida_mem;
	long neqs;
	int err;
	
	if (!ida || !(ida_mem = ida->mem)) {
		return MPT_ERROR(BadArgument);
	}
	if ((neqs = ida->ivp.neqs) < 1 || (neqs *= (ida->ivp.pint + 1)) <= 0) {
		return MPT_ERROR(BadArgument);
	}
	
	/* prepare initial vector */
	if (!ida->sd.y) {
		if (!(ida->sd.y = sundials_nvector_new(neqs))) {
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
	err = IDAInit(ida_mem, sundials_ida_fcn, ida->t, ida->sd.y, ida->yp);
	
	/* prepare tolerances */
	if (!ida->atol.base && !ida->rtol.base) {
		err = IDASStolerances(ida_mem, ida->rtol.d.val, ida->atol.d.val);
	} else {
		if (mpt_solver_module_tol_check(&ida->rtol, ida->ivp.neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_solver_module_tol_check(&ida->atol, ida->ivp.neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = IDAWFtolerances(ida_mem, sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	if (!ida->sd.stype) {
		ida->sd.stype = MPT_SOLVER_SUNDIALS(Direct);
	}
	if (!ida->sd.jacobian) {
		if (!ida->ivp.pint) {
			ida->sd.jacobian = SUNDIALS_DENSE;
		} else {
			ida->sd.jacobian = SUNDIALS_BAND;
			if (ida->sd.mu < 0) {
				ida->sd.mu = ida->ivp.neqs;
			}
			if (ida->sd.ml < 0) {
				ida->sd.ml = ida->ivp.neqs;
			}
		}
	}
	if ((err = sundials_linear(&ida->sd, neqs)) < 0) {
		return err;
	}
	if ((err = IDADlsSetLinearSolver(ida_mem, ida->sd.LS, ida->sd.A)) < 0) {
		return err;
	}
	if (ida->sd.A
	    && !(ida->sd.stype & MPT_SOLVER_SUNDIALS(Direct))
	    && ida->ufcn
	    && ida->ufcn->jac.fcn) {
		IDADlsSetJacFn(ida_mem, sundials_ida_jac);
	}
	return err;
}

