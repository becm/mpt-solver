/*!
 * sys4.c: solver setup for BACOL example
 */

#include "solver_ivp.h"

/* map functions to bacol parameters */
extern int user_init(MPT_SOLVER(IVP) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *log)
{
	int ret, npde = 4;
	
	(void) sd;
	if ((ret = mpt_object_set((void *) sol, "", "ii", npde, 0)) < 0) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s", "unable to set PDE count");
		return ret;
	}
	/* no profile operation */
	return 0;
}
