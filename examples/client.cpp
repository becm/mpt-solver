/*!
 *  define user init parameters
 */

#include "solver_run.h"

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(event.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(notify.h)
#include MPT_INCLUDE(solver.h)

class MyClient : public mpt::client
{
public:
	MyClient(mpt::solver::UserInit * = 0)
	{ }
	virtual ~MyClient()
	{ }
	void unref() __MPT_OVERRIDE
	{ delete this; }
	
	int dispatch(mpt::event *ev) __MPT_OVERRIDE
	{ return mpt::solver::mpt_solver_dispatch(this, ev); }
	
	int process(uintptr_t , mpt::iterator *) __MPT_OVERRIDE
	{ return mpt::event::Terminate; }
};

int main(int argc, char * const argv[])
{
	if (mpt::mpt_init(argc, argv) < 0) {
		return 1;
	}
	return mpt::solver_run(new MyClient);
}
