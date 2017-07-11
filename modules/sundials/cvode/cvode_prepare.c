/*!
 * prepare CVode solver descriptor for step operation
 */

#include <string.h>

#include <cvode/cvode_spgmr.h>
#include <cvode/cvode_spbcgs.h>
#include <cvode/cvode_sptfqmr.h>

#include <cvode/cvode_dense.h>
#include <cvode/cvode_diag.h>
#include <cvode/cvode_band.h>

#include <cvode/cvode_impl.h>

#define _SUNDIALS_GENERIC_TYPE(x) void
#include "sundials.h"

#ifdef SUNDIALS_WITH_LAPACK
# include <cvode/cvode_lapack.h>
/* set lapack solver jacobian method */
static int setLapack(CVodeMem cv, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	switch (sd->jacobian) {
		case MPT_SOLVER_ENUM(SundialsJacDiag):
		case MPT_SOLVER_ENUM(SundialsJacDiag) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return CVDiag(cv);
		case MPT_SOLVER_ENUM(SundialsJacDense):
			if (CVDense(cv, neqs)) {
				return -1;
			}
			return CVDlsSetDenseJacFn(cv, sundials_cvode_jac_dense);
		
		case MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return CVDense(cv, neqs);
		
		case MPT_SOLVER_ENUM(SundialsJacBand):
			if (CVBand(cv, neqs, sd->mu, sd->ml)) {
				return -1;
			}
			return CVDlsSetBandJacFn(cv, sundials_cvode_jac_band);
		
		case MPT_SOLVER_ENUM(SundialsJacBand) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return CVBand(cv, neqs, sd->mu, sd->ml);
		default:
			return -1;
	}
}
#endif
/* set direct solver jacobian method */
static int setDls(CVodeMem cv, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	switch (sd->jacobian) {
		case MPT_SOLVER_ENUM(SundialsJacDiag):
		case MPT_SOLVER_ENUM(SundialsJacDiag) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return CVDiag(cv);
		case MPT_SOLVER_ENUM(SundialsJacDense):
			if (CVDense(cv, neqs)) {
				return -1;
			}
			return CVDlsSetDenseJacFn(cv, sundials_cvode_jac_dense);
		
		case MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return CVDense(cv, neqs);
		
		case MPT_SOLVER_ENUM(SundialsJacBand):
			if (CVBand(cv, neqs, sd->mu, sd->ml)) {
				return -1;
			}
			return CVDlsSetBandJacFn(cv, sundials_cvode_jac_band);
		
		case MPT_SOLVER_ENUM(SundialsJacBand) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return CVBand(cv, neqs, sd->mu, sd->ml);
		default:
			return -1;
	}
}

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
		if (mpt_solver_tol_check(&cv->rtol, cv->ivp.neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_solver_tol_check(&cv->atol, cv->ivp.neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = CVodeWFtolerances(cv_mem, sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	if (!cv->sd.jacobian) cv->sd.linalg = 0;
	else if (!cv->sd.linalg) cv->sd.linalg = MPT_SOLVER_ENUM(SundialsDls);
	
	if (cv->ivp.pint && cv->sd.jacobian) {
		cv->sd.jacobian |= MPT_SOLVER_ENUM(SundialsJacNumeric);
	}
	switch (cv->sd.linalg) {
	  case 0: return CVodeSetIterType(cv_mem, CV_FUNCTIONAL);
	  case MPT_SOLVER_ENUM(SundialsDls):        return setDls(cv_mem, &cv->sd, neqs);
	  case MPT_SOLVER_ENUM(SundialsSpilsGMR):   return CVSpgmr(cv_mem, PREC_BOTH, 0);
	  case MPT_SOLVER_ENUM(SundialsSpilsBCG):   return CVSpbcg(cv_mem, PREC_BOTH, 0);
	  case MPT_SOLVER_ENUM(SundialsSpilsTFQMR): return CVSptfqmr(cv_mem, PREC_BOTH, 0);
#ifdef SUNDIALS_WITH_LAPACK
	  case MPT_SOLVER_ENUM(SundialsLapack):     return setLapack(cv_mem, &cv->sd, neqs);
#endif
	  default: return -1;
	}
}
