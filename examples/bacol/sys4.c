/*!
 * sys4.c: solver setup for BACOL example
 */

#include "solver_run.h"

/* map functions to bacol parameters */
static int bacol_init(const MPT_INTERFACE(metatype) *sol, MPT_STRUCT(solver_data) *sd, MPT_INTERFACE(logger) *log)
{
	int ret, npde = 4;
	
	(void) sd;
	if ((ret = mpt_init_pde(sol, 0, npde, &sd->val, log)) < 0) {
		return ret;
	}
	/* no profile operation */
	return 0;
}

int main(int argc, char * const argv[])
{
	MPT_INTERFACE(client) *cl;
	if (mpt_init(argc, argv) < 0) {
		return 1;
	}
	cl  = mpt_client_ivp(bacol_init);
	return solver_run(cl);
}
