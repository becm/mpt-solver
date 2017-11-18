/*!
 * main routine for MPT solver examples
 */

#include <stdlib.h>

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

static int dispatchClient(void *ptr, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(client) *cl = ptr;
	if (!ev) {
		cl->_vptr->meta.ref.unref((void *) cl);
		return 0;
	}
	return cl->_vptr->dispatch(cl, ev);
}

extern int solver_run(MPT_INTERFACE(client) *c)
{
	MPT_STRUCT(notify) no = MPT_NOTIFY_INIT;
	MPT_INTERFACE(logger) *info;
	MPT_INTERFACE(config) *cfg;
	MPT_STRUCT(dispatch) *disp;
	int ret;
	
	/* prefer solver client log output */
	info = 0;
	ret = c->_vptr->meta.conv((void *) c, MPT_ENUM(TypeLogger), &info);
	
	/* set sovler client config */
	cfg = 0;
	if ((ret = c->_vptr->meta.conv((void *) c, MPT_ENUM(TypeConfig), &cfg)) > 0
	    && cfg) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		
		/* set global config root */
		val.ptr = "mpt.client";
		cfg->_vptr->assign(cfg, 0, &val);
		
		/* set solver client arguments */
		if ((ret = mpt_client_config(cfg)) < 0) {
			c->_vptr->meta.ref.unref((void *) c);
			return EXIT_FAILURE;
		}
	}
	/* register message dispatcher for notifier */
	if (!(disp = mpt_notify_dispatch(&no))) {
		c->_vptr->meta.ref.unref((void *) c);
		mpt_log(info, __func__, MPT_LOG(Error), "%s", "failed to create dispatcher");
		return 2;
	}
	/* setup dispatcher for solver client */
	if (mpt_dispatch_set(disp, MPT_MESGTYPE(Command), dispatchClient, c) < 0) {
		dispatchClient(c, 0);
		mpt_log(info, __func__, MPT_LOG(Error), "%s", "event setup failed");
		return 3;
	}
	/* default event for detached run */
	if (!no._fdused) {
		disp->_def = MPT_MESGTYPE(Command);
	}
	/* start event loop */
	mpt_loop(&no);
	
	mpt_notify_fini(&no);
	
	return 0;
}

