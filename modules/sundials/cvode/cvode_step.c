/*!
 * execute CVode solver step.
 */

#include <cvode/cvode.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode step operation
 * 
 * Execute CVode solver step to requested end.
 * 
 * \param cv   CVode solver data
 * \param tend step target time
 * 
 * \return non-zero on error
 */
extern int mpt_sundials_cvode_step(MPT_SOLVER_STRUCT(cvode) *cv, double tend)
{
	int err;
	
	if (!cv->sd.y) {
		return CV_MEM_NULL;
	}
	/* zero step limit indicates single step mode */
	err = cv->mxstep ? CV_NORMAL : CV_ONE_STEP;
	
	err = CVode(cv->mem, tend, cv->sd.y, &cv->t, err);
	return err < 0 ? err : 0;
}

