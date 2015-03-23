/*!
 * prepare IDA solver
 */

#include <string.h>
#include <errno.h>

#include <ida/ida_spgmr.h>
#include <ida/ida_spbcgs.h>
#include <ida/ida_sptfqmr.h>

#include <ida/ida_dense.h>
#include <ida/ida_band.h>

#include <ida/ida_impl.h>

#include "sundials.h"

#ifdef SUNDIALS_WITH_LAPACK
# include <ida/ida_lapack.h>
/* set lapack solver jacobian method */
static int setLapack(IDAMem ida, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	int err;
	switch (sd->jacobian) {
		case MPT_ENUM(SundialsJacBand):
			if ((err = IDALapackBand(ida, neqs, sd->mu, sd->ml))) {
				return -1;
			}
			return IDADlsSetBandJacFn(ida, sundials_ida_jac_band);
		
		case MPT_ENUM(SundialsJacBand) | MPT_ENUM(SundialsJacNumeric):
			return IDALapackBand(ida, neqs, sd->mu, sd->ml);
		
		case MPT_ENUM(SundialsJacDense):
			if ((err = IDALapackDense(ida, neqs))) {
				return -1;
			}
			return IDADlsSetDenseJacFn(ida, sundials_ida_jac_dense);
		default:
			sd->jacobian = MPT_ENUM(SundialsJacDense) | MPT_ENUM(SundialsJacNumeric);
			return IDALapackDense(ida, neqs);
	}
}
#endif

/* set direct linear solver jacobian method */
static int setDls(IDAMem ida, MPT_SOLVER_STRUCT(sundials) *sd, long neqs)
{
	int err;
	switch (sd->jacobian) {
		case MPT_ENUM(SundialsJacBand):
			if ((err = IDABand(ida, neqs, sd->mu, sd->ml))) {
				return -1;
			}
			return IDADlsSetBandJacFn(ida, sundials_ida_jac_band);
		
		case MPT_ENUM(SundialsJacBand) | MPT_ENUM(SundialsJacNumeric):
			return IDABand(ida, neqs, sd->mu, sd->ml);
		
		case MPT_ENUM(SundialsJacDense):
			if ((err = IDADense(ida, neqs))) {
				return -1;
			}
			return IDADlsSetDenseJacFn(ida, sundials_ida_jac_dense);
		default:
			sd->jacobian = MPT_ENUM(SundialsJacDense) | MPT_ENUM(SundialsJacNumeric);
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
extern int sundials_ida_prepare(MPT_SOLVER_STRUCT(ida) *data, double *val)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp;
	IDAMem ida_mem;
	long neqs;
	int err;
	
	if (!(ivp = &data->ivp) || !(ida_mem = data->mem) || !val) {
		errno = EFAULT;
		return IDA_MEM_NULL;
	}
	
	if ((neqs = ivp->neqs) < 1 || (neqs *= (ivp->pint + 1)) <= 0) {
		return -2;
	}
	
	/* prepare initial vector */
	if (!data->sd.y && !(data->sd.y = sundials_nvector_empty(neqs))) {
		return IDA_MEM_NULL;
	}
	if (!data->yp && !(data->yp = N_VClone(data->sd.y))) {
		return IDA_MEM_NULL;
	}
	N_VSetArrayPointer(val, data->sd.y);
	err = IDAInit(ida_mem, sundials_ida_fcn, ivp->last, data->sd.y, data->yp);
	N_VSetArrayPointer(0, data->sd.y);
	
	/* prepare tolerances */
	if (!data->atol.base && !data->rtol.base) {
		err = IDASStolerances(ida_mem, data->rtol.d.val, data->atol.d.val);
	} else {
		if (mpt_vecpar_cktol(&data->rtol, ivp->neqs, 1, __MPT_IVP_RTOL) < 0) {
			return -21;
		}
		if (mpt_vecpar_cktol(&data->atol, ivp->neqs, 1, __MPT_IVP_ATOL) < 0) {
			return -22;
		}
		err = IDAWFtolerances(ida_mem, sundials_ewtfcn);
	}
	if (err < 0) {
		return err;
	}
	switch (data->sd.linalg) {
	  case 0: data->sd.linalg = MPT_ENUM(SundialsDls);
	  case MPT_ENUM(SundialsDls):        return setDls(ida_mem, &data->sd, neqs);
	  case MPT_ENUM(SundialsSpilsGMR):   return IDASpgmr(ida_mem, 0);
	  case MPT_ENUM(SundialsSpilsBCG):   return IDASpbcg(ida_mem, 0);
	  case MPT_ENUM(SundialsSpilsTFQMR): return IDASptfqmr(ida_mem, 0);
#ifdef SUNDIALS_WITH_LAPACK
	  case MPT_ENUM(SundialsLapack):     return setLapack(ida_mem, &data->sd, neqs);
#endif
	  default: return -1;
	}
}

