/*!
 *  define user init parameters
 */

#include "solver_run.h"

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(config.h)

class MyClient : public mpt::client
{
public:
	virtual ~MyClient()
	{ }
	void unref() __MPT_OVERRIDE
	{ delete this; }
	
	int step(mpt::iterator *) __MPT_OVERRIDE
	{ return 0; }
	int init(mpt::iterator *) __MPT_OVERRIDE
	{ return 0; }
};

int main(int argc, char * const argv[])
{
	if (mpt::client_init(argc, argv) < 0) {
		return 1;
	}
	mpt::mpt_config_set(0, "mpt.client", "", '.');
	mpt::mpt_config_set(0, "mpt.client.solconf", "", '.');
	return mpt::solver_run(new MyClient);
}
