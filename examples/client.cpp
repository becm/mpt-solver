/*!
 *  define user init parameters
 */

#include <iostream>

#include "solver_run.h"

#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(event.h)

#include MPT_INCLUDE(solver.h)

class client : public mpt::client
{
public:
	client()
	{ }
	virtual ~client()
	{ }
	void unref() __MPT_OVERRIDE
	{
		std::cout << __func__ << std::endl;
		delete this;
	}
	int conv(int type, void *ptr) const __MPT_OVERRIDE
	{
		std::cout << __func__ << " " << type << std::endl;
		if (type == mpt::typeinfo<mpt::config *>::id()) {
			return mpt::BadType;
		}
		return client::conv(type, ptr);
	}
	int dispatch(mpt::event *ev) __MPT_OVERRIDE
	{
		if (!ev) {
			std::cout << __func__ << " (term)" << std::endl;
			return mpt::event::Terminate;
		}
		std::cout << __func__ << " (solver)" << std::endl;
		return mpt::solver::mpt_solver_dispatch(this, ev);
	}
	int process(uintptr_t id, mpt::iterator *) __MPT_OVERRIDE
	{
		std::cout << __func__ << " " << id << std::endl;
		if (!id) {
			return mpt::event::Default;
		}
		return mpt::event::None;
	}
};

int main(int argc, char * const argv[])
{
	if (mpt::mpt_init(argc, argv) < 0) {
		return 1;
	}
	return mpt::solver_run(new client);
}
