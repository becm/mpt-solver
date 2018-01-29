/*!
 * prepare CVode solver descriptor for step operation
 */

#include <string.h>

#include <sunlinsol/sunlinsol_spgmr.h>
#include <sunlinsol/sunlinsol_spbcgs.h>
#include <sunlinsol/sunlinsol_sptfqmr.h>

#include <stdio.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunlinsol/sunlinsol_band.h>

#include <cvode/cvode_diag.h>
#include <cvode/cvode_impl.h>
#include <cvode/cvode_direct.h>

#ifdef SUNDIALS_BLAS_LAPACK
# include <sunlinsol/sunlinsol_lapackdense.h>
# include <sunlinsol/sunlinsol_lapackband.h>
#endif

#define _SUNDIALS_GENERIC_TYPE(x) void
#include "sundials.h"

static int setLS(MPT_SOLVER_STRUCT(sundials) *sd, SUNLinearSolver LS, SUNMatrix A)
{
	if (sd->A) {
		SUNMatDestroy(sd->A);
	}
	if (sd->LS) {
		SUNLinSolFree(sd->LS);
	}
	sd->A = A;
	sd->LS = LS;
	
	return 0;
}

#ifdef SUNDIALS_BLAS_LAPACK
/* set lapack solver jacobian method */
static int setLapack(MPT_SOLVER_STRUCT(sundials) *sd, sunindextype neqs)
{
	SUNMatrix A;
	SUNLinearSolver LS;
	int jac;
	
	jac = sd->jacobian & ~MPT_SOLVER_ENUM(SundialsJacNumeric);
	
	if (jac == MPT_SOLVER_ENUM(SundialsJacDense)) {
		if (!(A = SUNDenseMatrix(neqs, neqs))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNLapackDense(sd->y, A))) {
			SUNMatDestroy(A);
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, LS, A);
	}
	if (jac == MPT_SOLVER_ENUM(SundialsJacBand)) {
		sunindextype smu = sd->mu + sd->ml;
		if (smu >= neqs) {
			smu = neqs - 1;
		}
		if (!(A = SUNBandMatrix(neqs, sd->mu, sd->ml, smu))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNLapackBand(sd->y, A))) {
			SUNMatDestroy(A);
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, LS, A);
	}
	return MPT_ERROR(BadArgument);
}
#endif

/* set direct solver jacobian method */
static int setDls(MPT_SOLVER_STRUCT(sundials) *sd, sunindextype neqs)
{
	SUNMatrix A;
	SUNLinearSolver LS;
	int jac;
	
	jac = sd->jacobian & ~MPT_SOLVER_ENUM(SundialsJacNumeric);
	
	if (jac == MPT_SOLVER_ENUM(SundialsJacDense)) {
		if (!(A = SUNDenseMatrix(neqs, neqs))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNDenseLinearSolver(sd->y, A))) {
			SUNMatDestroy(A);
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, LS, A);
	}
	if (jac == MPT_SOLVER_ENUM(SundialsJacBand)) {
		sunindextype smu = sd->mu + sd->ml;
		if (smu >= neqs) {
			smu = neqs - 1;
		}
		if (!(A = SUNBandMatrix(neqs, sd->mu, sd->ml, smu))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNBandLinearSolver(sd->y, A))) {
			SUNMatDestroy(A);
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, LS, A);
	}
	return MPT_ERROR(BadArgument);
}

/*!
 * \ingroup mptSundials
 * \brief setup SUNDIALS linear solver
 * 
 * Prepare SUNDIALS solver for step operation.
 * 
 * \param data  CVode solver data
 * \param val   initial values
 * 
 * \return non-zero on error
 */
extern int sundials_linear(MPT_SOLVER_STRUCT(sundials) *sd, sunindextype neqs)
{
	SUNLinearSolver s;
	
	if (sd->linalg == MPT_SOLVER_ENUM(SundialsDls)) {
		return setDls(sd, neqs);
	}
	if (sd->linalg == MPT_SOLVER_ENUM(SundialsSpilsGMR)) {
		if (!(s = SUNSPGMR(sd->y, sd->prec, sd->kmax))) {
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, s, 0);
	}
	if (sd->linalg == MPT_SOLVER_ENUM(SundialsSpilsBCG)) {
		if (!(s = SUNSPBCGS(sd->y, sd->prec, sd->kmax))) {
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, s, 0);
	}
	if (sd->linalg == MPT_SOLVER_ENUM(SundialsSpilsTFQMR)) {
		if (!(s = SUNSPTFQMR(sd->y, sd->prec, sd->kmax))) {
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, s, 0);
	}
#ifdef SUNDIALS_BLAS_LAPACK
	if (sd->linalg == MPT_SOLVER_ENUM(SundialsLapack)) {
		return setLapack(sd, neqs);
	}
#endif
	return MPT_ERROR(BadArgument);
}
