/*!
 *  define user init parameters
 */

#include "solver_run.h"

#include MPT_INCLUDE(notify.h)
#include MPT_INCLUDE(output.h)

int main(int argc, char * const argv[])
{
	MPT_INTERFACE(metatype) *out;
	MPT_INTERFACE(client) *cl;
	if (mpt_init(argc, argv) < 0) {
		return 1;
	}
	out = mpt_output_local();
	cl  = mpt_client_nls(user_init, out);
	return solver_run(cl);
}
