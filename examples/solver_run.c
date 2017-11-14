/*!
 * main routine for MPT solver examples
 */

#include <stdio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(event.h)
#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(config.h)

#include MPT_INCLUDE(notify.h)

#include MPT_INCLUDE(solver.h)

#include "solver_run.h"

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

static MPT_STRUCT(notify) no = MPT_NOTIFY_INIT;
static char * const *args = 0;

extern int client_init(int argc, char * const argv[])
{
	int pos;
	mtrace();
	
	/* environment and connection setup */
	if ((pos = mpt_init(&no, argc, argv)) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s", "client init failed");
		return -1;
	}
	/* non-mpt arguments */
	if (pos < argc) {
		args = argv + pos;
		return argc - pos;
	}
	return 0;
}

extern int solver_run(MPT_INTERFACE(client) *c)
{
	MPT_INTERFACE(config) *cfg;
	MPT_STRUCT(dispatch) *disp;
	
	/* set client config root */
	cfg = 0;
	if (c->_vptr->meta.conv((void *) c, MPT_ENUM(TypeConfig), &cfg) > 0
	    && cfg) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		val.ptr = "mpt.client";
		cfg->_vptr->assign(cfg, 0, &val);
	}
	if (!(disp = mpt_notify_dispatch(&no))) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s", "failed to create dispatcher");
		return MPT_ERROR(BadOperation);
	}
	/* setup dispatcher for solver client */
	if (mpt_meta_events(disp, (void *) c) < 0) {
		mpt_log(0, __func__, MPT_LOG(Error), "%s", "event setup failed");
		return MPT_ERROR(BadValue);
	}
	/* default event for detached run */
	if (!no._fdused) {
		disp->_def = MPT_MESGTYPE(Command);
	}
	/* set solver client arguments */
	if (cfg && args) {
		int take = mpt_solver_args(cfg, args, -1);
		if (take < 0) {
			return 1;
		}
		args += take;
	}
	/* start event loop */
	mpt_loop(&no);
	
	mpt_notify_fini(&no);
	
	return 0;
}

