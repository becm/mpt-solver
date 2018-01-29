/*!
 * execute IDA solver integration step
 */

#include <ida/ida.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief IDA step operation
 * 
 * Execute IDA solver step to requested end.
 * 
 * \param ida  IDA solver data
 * \param tend step target time
 * 
 * \return non-zero on error
 */
extern int sundials_ida_step(MPT_SOLVER_STRUCT(ida) *ida, double tend)
{
	int err;
	
	if (!ida->sd.y || !ida->yp) {
		return MPT_ERROR(BadOperation);
	}
	if (!(err = ida->sd.step)) {
		err = IDA_NORMAL;
	}
	err = IDASolve(ida->mem, tend, &ida->t, ida->sd.y, ida->yp, err);
	
	return err < 0 ? err : 0;
}
