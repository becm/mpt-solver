/*!
 * prepare CVode solver descriptor for step operation
 */

#include <string.h>
#include <stdio.h>

#include <sunlinsol/sunlinsol_spgmr.h>
#include <sunlinsol/sunlinsol_spbcgs.h>
#include <sunlinsol/sunlinsol_sptfqmr.h>

#include <sunlinsol/sunlinsol_dense.h>
#include <sunlinsol/sunlinsol_band.h>

#ifdef MPT_WITH_LAPACK
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

#ifdef MPT_WITH_LAPACK
/* set lapack solver jacobian method */
static int setLapack(MPT_SOLVER_STRUCT(sundials) *sd, sunindextype neqs)
{
	SUNMatrix A;
	SUNLinearSolver LS;
	
	if (sd->jacobian == SUNDIALS_DENSE) {
		if (!(A = SUNDenseMatrix(neqs, neqs))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNLinSol_LapackDense(sd->y, A))) {
			SUNMatDestroy(A);
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, LS, A);
	}
	if (sd->jacobian == SUNDIALS_BAND) {
		/* direct solver; matrix WILL be LU factored! */
		if (!(A = SUNBandMatrix(neqs, sd->mu, sd->ml))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNLinSol_LapackBand(sd->y, A))) {
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
	
	if (sd->jacobian == SUNDIALS_DENSE) {
		if (!(A = SUNDenseMatrix(neqs, neqs))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNLinSol_Dense(sd->y, A))) {
			SUNMatDestroy(A);
			return MPT_ERROR(BadOperation);
		}
		return setLS(sd, LS, A);
	}
	if (sd->jacobian == SUNDIALS_BAND) {
		/* direct solver; matrix WILL be LU factored! */
		if (!(A = SUNBandMatrix(neqs, sd->mu, sd->ml))) {
			return MPT_ERROR(BadValue);
		}
		if (!(LS = SUNLinSol_Band(sd->y, A))) {
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
extern int mpt_sundials_linear(MPT_SOLVER_STRUCT(sundials) *sd, sunindextype neqs)
{
	SUNLinearSolver s;
	
	switch (sd->linsol & ~MPT_SOLVER_SUNDIALS(Numeric)) {
		case MPT_SOLVER_SUNDIALS(Direct):
			return setDls(sd, neqs);
		case MPT_SOLVER_SUNDIALS(IterGMR):
			if (!(s = SUNLinSol_SPGMR(sd->y, sd->prec, sd->kmax))) {
				return MPT_ERROR(BadOperation);
			}
			return setLS(sd, s, 0);
		case MPT_SOLVER_SUNDIALS(IterBCG):
			if (!(s = SUNLinSol_SPBCGS(sd->y, sd->prec, sd->kmax))) {
				return MPT_ERROR(BadOperation);
			}
			return setLS(sd, s, 0);
		case MPT_SOLVER_SUNDIALS(IterTFQMR):
			if (!(s = SUNLinSol_SPTFQMR(sd->y, sd->prec, sd->kmax))) {
				return MPT_ERROR(BadOperation);
			}
			return setLS(sd, s, 0);
#ifdef MPT_WITH_LAPACK
		case MPT_SOLVER_SUNDIALS(Lapack):
			return setLapack(sd, neqs);
#endif
		default:
			return MPT_ERROR(BadArgument);
	}
}
