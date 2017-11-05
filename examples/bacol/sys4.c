/*!
 * sys4.c: solver setup for BACOL example
 */

#include "solver_run.h"

/* map functions to bacol parameters */
extern int user_init(MPT_SOLVER(interface) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *log)
{
	int ret, npde = 4;
	
	(void) sd;
	if ((ret = mpt_init_pde(sol, 0, npde, &sd->val, log)) < 0) {
		return ret;
	}
	/* no profile operation */
	return 0;
}
