/*!
 *  define user init parameters
 */

#include "solver_run.h"

#include MPT_INCLUDE(output.h)

int main(int argc, char * const argv[])
{
	MPT_INTERFACE(output) *out;
	if (client_init(argc, argv) < 0) {
		return 1;
	}
	out = mpt_output_local();
	return solver_run(mpt_client_ivp(out, user_init));
}
