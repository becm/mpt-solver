/*!
 * execute CVode solver step.
 */

#include <errno.h>

#include <cvode/cvode.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode step operation
 * 
 * Execute CVode solver step to requested end.
 * 
 * \param data  CVode solver data
 * \param val   current values
 * \param tend  step target time
 * 
 * \return non-zero on error
 */
extern int sundials_cvode_step(MPT_SOLVER_STRUCT(cvode) *data, double *val, double tend)
{
	int err;
	
	if (!data->sd.y) {
		errno = EFAULT;
		return CV_MEM_NULL;
	}
	err = (data->sd.step & MPT_ENUM(SundialsStepSingle)) ? CV_ONE_STEP : CV_NORMAL;
	
	N_VSetArrayPointer(val, data->sd.y);
	err = CVode(data->mem, tend, data->sd.y, &data->ivp.last, err);
	N_VSetArrayPointer(0, data->sd.y);
	
	return err < 0 ? err : 0;
}

