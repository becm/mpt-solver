/*!
 * prepare CVode solver descriptor for step operation
 */

#include <string.h>
#include <errno.h>

#include <cvode/cvode_spgmr.h>
#include <cvode/cvode_spbcgs.h>
#include <cvode/cvode_sptfqmr.h>

#include <cvode/cvode_dense.h>
#include <cvode/cvode_diag.h>
#include <cvode/cvode_band.h>

#include <cvode/cvode_impl.h>

#include "sundials.h"

#ifdef SUNDIALS_WITH_LAPACK
# include <cvode/cvode_lapack.h>
/* set lapack solver jacobian method */
static int setLapack(CVodeMem cv, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	switch (sd->jacobian) {
		case MPT_ENUM(SundialsJacDiag):
		case MPT_ENUM(SundialsJacDiag) | MPT_ENUM(SundialsJacNumeric):
			return CVDiag(cv);
		case MPT_ENUM(SundialsJacDense):
			if (CVDense(cv, neqs)) {
				return -1;
			}
			return CVDlsSetDenseJacFn(cv, sundials_cvode_jac_dense);
		
		case MPT_ENUM(SundialsJacDense) | MPT_ENUM(SundialsJacNumeric):
			return CVDense(cv, neqs);
		
		case MPT_ENUM(SundialsJacBand):
			if (CVBand(cv, neqs, sd->mu, sd->ml)) {
				return -1;
			}
			return CVDlsSetBandJacFn(cv, sundials_cvode_jac_band);
		
		case MPT_ENUM(SundialsJacBand) | MPT_ENUM(SundialsJacNumeric):
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
		case MPT_ENUM(SundialsJacDiag):
		case MPT_ENUM(SundialsJacDiag) | MPT_ENUM(SundialsJacNumeric):
			return CVDiag(cv);
		case MPT_ENUM(SundialsJacDense):
			if (CVDense(cv, neqs)) {
				return -1;
			}
			return CVDlsSetDenseJacFn(cv, sundials_cvode_jac_dense);
		
		case MPT_ENUM(SundialsJacDense) | MPT_ENUM(SundialsJacNumeric):
			return CVDense(cv, neqs);
		
		case MPT_ENUM(SundialsJacBand):
			if (CVBand(cv, neqs, sd->mu, sd->ml)) {
				return -1;
			}
			return CVDlsSetBandJacFn(cv, sundials_cvode_jac_band);
		
		case MPT_ENUM(SundialsJacBand) | MPT_ENUM(SundialsJacNumeric):
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
extern int sundials_cvode_prepare(MPT_SOLVER_STRUCT(cvode) *data, double *val)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	CVodeMem cv_mem;
	long neqs;
	int err;
	
	if (!(ivp = &data->ivp) || !(cv_mem = data->mem) || !val) {
		errno = EFAULT;
		return CV_MEM_NULL;
	}
	
	if ((neqs = ivp->neqs) < 1 || (neqs *= (ivp->pint + 1)) <= 0) {
		return -2;
	}
	
	/* prepare initial vector */
	if (data->sd.y) {
		N_VSetArrayPointer(0, data->sd.y);
		N_VDestroy(data->sd.y);
	}
	if (!(data->sd.y = sundials_nvector_empty(neqs))) {
		return -1;
	}
	N_VSetArrayPointer(val, data->sd.y);
	
	if ((err = CVodeInit(cv_mem, sundials_cvode_fcn, ivp->last, data->sd.y)) < 0) {
		return err;
	}
	/* prepare tolerances */
	if (!data->atol.base && !data->rtol.base) {
		err = CVodeSStolerances(cv_mem, data->rtol.d.val, data->atol.d.val);
	} else {
		if (mpt_vecpar_cktol(&data->rtol, ivp->neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_vecpar_cktol(&data->atol, ivp->neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = CVodeWFtolerances(cv_mem, sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	if (!data->sd.jacobian) data->sd.linalg = 0;
	else if (!data->sd.linalg) data->sd.linalg = MPT_ENUM(SundialsDls);
	
	switch (data->sd.linalg) {
	  case 0: return CVodeSetIterType(cv_mem, CV_FUNCTIONAL);
	  case MPT_ENUM(SundialsDls):        return setDls(cv_mem, &data->sd, neqs);
	  case MPT_ENUM(SundialsSpilsGMR):   return CVSpgmr(cv_mem, PREC_BOTH, 0);
	  case MPT_ENUM(SundialsSpilsBCG):   return CVSpbcg(cv_mem, PREC_BOTH, 0);
	  case MPT_ENUM(SundialsSpilsTFQMR): return CVSptfqmr(cv_mem, PREC_BOTH, 0);
#ifdef SUNDIALS_WITH_LAPACK
	  case MPT_ENUM(SundialsLapack):     return setLapack(cv_mem, &data->sd, neqs);
#endif
	  default: return -1;
	}
}
