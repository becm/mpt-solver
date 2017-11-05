/*!
 *  define user init parameters
 */

#include "solver_run.h"

#include MPT_INCLUDE(output.h)

int main(int argc, char * const argv[])
{
	MPT_INTERFACE(metatype) *out;
	MPT_INTERFACE(client) *cl;
	if (client_init(argc, argv) < 0) {
		return 1;
	}
	out = mpt_output_local();
	cl  = mpt_client_nls(out, user_init);
	return solver_run(cl);
}
