/*!
 * execute IDA solver integration step
 */

#include <errno.h>

#include <ida/ida.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief IDA step operation
 * 
 * Execute IDA solver step to requested end.
 * 
 * \param data  IDA solver data
 * \param val   current values
 * \param tend  step target time
 * 
 * \return non-zero on error
 */
extern int sundials_ida_step(MPT_SOLVER_STRUCT(ida) *data, double *val, double tend)
{
	int	err;
	
	if (!data->sd.y || !val) {
		errno = EFAULT;
		return -1;
	}
	N_VSetArrayPointer(val, data->sd.y);
	
	err = (data->sd.step & MPT_ENUM(SundialsStepSingle)) ? IDA_ONE_STEP : IDA_NORMAL;
	
	err = IDASolve(data->mem, tend, &data->ivp.last, data->sd.y, data->yp, err);
	
	return err < 0 ? err : 0;
}
