/*!
 * prepare IDA solver
 */

#include <string.h>

#include <ida/ida_spgmr.h>
#include <ida/ida_spbcgs.h>
#include <ida/ida_sptfqmr.h>

#include <ida/ida_dense.h>
#include <ida/ida_band.h>

#include <ida/ida_impl.h>

#define _SUNDIALS_GENERIC_TYPE(x) void
#include "sundials.h"

#ifdef SUNDIALS_WITH_LAPACK
# include <ida/ida_lapack.h>
/* set lapack solver jacobian method */
static int setLapack(IDAMem ida, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	int err;
	switch (sd->jacobian) {
		case MPT_SOLVER_ENUM(SundialsJacBand):
			if ((err = IDALapackBand(ida, neqs, sd->mu, sd->ml))) {
				return -1;
			}
			return IDADlsSetBandJacFn(ida, sundials_ida_jac_band);
		
		case MPT_SOLVER_ENUM(SundialsJacBand) | MPT_ENUM(SundialsJacNumeric):
			return IDALapackBand(ida, neqs, sd->mu, sd->ml);
		
		case MPT_SOLVER_ENUM(SundialsJacDense):
			if ((err = IDALapackDense(ida, neqs))) {
				return -1;
			}
			return IDADlsSetDenseJacFn(ida, sundials_ida_jac_dense);
		default:
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric);
			return IDALapackDense(ida, neqs);
	}
}
#endif

/* set direct linear solver jacobian method */
static int setDls(IDAMem ida, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	int err;
	switch (sd->jacobian) {
		case MPT_SOLVER_ENUM(SundialsJacBand):
			if ((err = IDABand(ida, neqs, sd->mu, sd->ml))) {
				return -1;
			}
			return IDADlsSetBandJacFn(ida, sundials_ida_jac_band);
		
		case MPT_SOLVER_ENUM(SundialsJacBand) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			return IDABand(ida, neqs, sd->mu, sd->ml);
		
		case MPT_SOLVER_ENUM(SundialsJacDense):
			if ((err = IDADense(ida, neqs))) {
				return -1;
			}
			return IDADlsSetDenseJacFn(ida, sundials_ida_jac_dense);
		default:
			sd->jacobian = MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric);
			return IDADense(ida, neqs);
	}
}

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
		realtype *y;
		if (!(ida->sd.y = sundials_nvector_new(neqs))) {
			return MPT_ERROR(BadOperation);
		}
		y = N_VGetArrayPointer(ida->sd.y);
		memset(y, 0, neqs * sizeof(*y));
	}
	if (!ida->yp && !(ida->yp = N_VClone(ida->sd.y))) {
		return MPT_ERROR(BadOperation);
	}
	err = IDAInit(ida_mem, sundials_ida_fcn, ida->t, ida->sd.y, ida->yp);
	
	/* prepare tolerances */
	if (!ida->atol.base && !ida->rtol.base) {
		err = IDASStolerances(ida_mem, ida->rtol.d.val, ida->atol.d.val);
	} else {
		if (mpt_vecpar_cktol(&ida->rtol, ida->ivp.neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_vecpar_cktol(&ida->atol, ida->ivp.neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = IDAWFtolerances(ida_mem, sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	if (ida->ivp.pint && ida->sd.jacobian) {
		ida->sd.jacobian |= MPT_SOLVER_ENUM(SundialsJacNumeric);
	}
	switch (ida->sd.linalg) {
	  case 0: ida->sd.linalg = MPT_SOLVER_ENUM(SundialsDls);
	  case MPT_SOLVER_ENUM(SundialsDls):        return setDls(ida_mem, &ida->sd, neqs);
	  case MPT_SOLVER_ENUM(SundialsSpilsGMR):   return IDASpgmr(ida_mem, 0);
	  case MPT_SOLVER_ENUM(SundialsSpilsBCG):   return IDASpbcg(ida_mem, 0);
	  case MPT_SOLVER_ENUM(SundialsSpilsTFQMR): return IDASptfqmr(ida_mem, 0);
#ifdef SUNDIALS_WITH_LAPACK
	  case MPT_SOLVER_ENUM(SundialsLapack):     return setLapack(ida_mem, &data->sd, neqs);
#endif
	  default: return -1;
	}
}

