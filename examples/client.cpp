/*!
 *  define user init parameters
 */

#include "solver_run.h"

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(client.h)

class MyClient : public mpt::client
{
public:
	virtual ~MyClient()
	{ }
	void unref() __MPT_OVERRIDE
	{ delete this; }
	
	int assign(const mpt::path *p, const mpt::value *v) __MPT_OVERRIDE;
	
	int step(mpt::iterator *) __MPT_OVERRIDE
	{ return 0; }
	int init(mpt::iterator *) __MPT_OVERRIDE
	{ return 0; }
};
int MyClient::assign(const mpt::path *p, const mpt::value *v)
{
	int ret;
	
	if (!p) {
		return v ? mpt::BadArgument : 0;
	}
	ret = mpt::client::assign(p, v);
	
	// simulate solver config existance
	if (p->empty()) {
		set("solconf", "");
	}
	return ret;
}

int main(int argc, char * const argv[])
{
	if (mpt::client_init(argc, argv) < 0) {
		return 1;
	}
	return mpt::solver_run(new MyClient);
}
