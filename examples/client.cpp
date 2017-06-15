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
	
	mpt::metatype *query(const mpt::path *) const __MPT_OVERRIDE
	{ return 0; }
	int assign(const mpt::path *, const mpt::value *) __MPT_OVERRIDE
	{ return 0; }
	int remove(const mpt::path *) __MPT_OVERRIDE
	{ return 0; }
	
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
	return mpt::solver_run(new MyClient);
}
