/*!
 * sys4.c: solver setup for BACOL example
 */

#include <stdlib.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# error: bad
# define mtrace()
#endif

#include "solver_run.h"

/* solver/client setup for PDE run */
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
	mtrace();
	if (mpt_init(argc, argv) < 0) {
		return 1;
	}
	cl  = mpt_client_ivp(bacol_init);
	return solver_run(cl);
}
