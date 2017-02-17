/*!
 *  define user init parameters
 */
#include "solver_nls.h"

int main(int argc, char * const argv[])
{
	if (client_init(argc, argv) < 0) return 1;
	return solver_run(mpt_client_nls(user_init, "mpt.client"));
}
