/*!
 *  define user init parameters
 */

#include "solver_run.h"

#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(config.h)
#include MPT_INCLUDE(event.h)
#include MPT_INCLUDE(solver.h)

class MyClient : public mpt::client
{
public:
    MyClient()
    { }
    virtual ~MyClient()
    { }
    void unref() __MPT_OVERRIDE
    {
        delete this;
    }
    int conv(int type, void *ptr) const __MPT_OVERRIDE
    {
        if (type == mpt::config::Type) {
            return mpt::BadType;
        }
        return client::conv(type, ptr);
    }
    int dispatch(mpt::event *ev) __MPT_OVERRIDE
    {
        if (!ev) {
            return mpt::event::Terminate;
        }
        return mpt::solver::mpt_solver_dispatch(this, ev);
    }
    int process(uintptr_t id, mpt::iterator *) __MPT_OVERRIDE
    {
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
    return mpt::solver_run(new MyClient);
}
