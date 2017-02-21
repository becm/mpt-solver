/*!
 *  define user init parameters
 */
#include "solver_nls.h"

#include MPT_INCLUDE(output.h)

int main(int argc, char * const argv[])
{
	MPT_INTERFACE(output) *out;
	if (client_init(argc, argv) < 0) return 1;
	out = mpt_output_local();
	return solver_run(mpt_client_nls(out, user_init, "mpt.client"));
}
