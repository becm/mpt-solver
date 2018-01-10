/*!
 * main routine for MPT solver examples
 */

#include "solver_run.h"

#include MPT_INCLUDE(client.h)
#include MPT_INCLUDE(message.h)
#include MPT_INCLUDE(config.h)

#include MPT_INCLUDE(notify.h)

#include MPT_INCLUDE(solver.h)

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
	MPT_INTERFACE(config) *cfg;
	int ret;
	
	/* set solver client config */
	cfg = 0;
	if ((ret = c->_vptr->meta.conv((void *) c, MPT_ENUM(TypeConfig), &cfg)) > 0
	    && cfg) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		
		/* set global config root */
		val.ptr = "mpt.client";
		cfg->_vptr->assign(cfg, 0, &val);
		
		/* set solver client arguments */
		if ((ret = mpt_client_config(cfg)) < 0) {
			dispatchClient(c, 0);
			return 1;
		}
	}
	/* global controller setup */
	ret = mpt_notify_config(&no, 0);
	
	/* remote run */
	if (no._fdused) {
		MPT_STRUCT(dispatch) *disp;
		
		/* register message dispatcher for notifier */
		if (!(disp = mpt_notify_dispatch(&no))) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s",
			        "failed to create dispatcher");
			dispatchClient(c, 0);
			ret = 2;
		}
		/* setup dispatcher for solver client */
		else if ((ret = mpt_dispatch_set(disp, MPT_MESGTYPE(Command), dispatchClient, c)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s",
			        "event setup failed");
			dispatchClient(c, 0);
			ret = 3;
		}
		/* execute event loop */
		else {
			mpt_dispatch_param(disp, 0);
			ret = mpt_loop(&no);
		}
	}
	else {
		/* interactive file assignment */
		if (cfg && (ret = mpt_solver_require(cfg, 0)) < 0) {
			ret = 1;
		}
		/* setup standalone operation */
		else if ((ret = c->_vptr->process(c, 0, 0)) < 0) {
			mpt_log(0, __func__, MPT_LOG(Error), "%s",
			        "solver client start failed");
			ret = 2;
		}
		/* execute standalone run */
		else if (ret & MPT_EVENTFLAG(Default)) {
			while ((ret = c->_vptr->dispatch(c, 0))) {
				if (ret < 0) {
					ret = 3;
					break;
				}
				if (ret & MPT_EVENTFLAG(Terminate)) {
					ret = 0;
					break;
				}
			}
		}
		dispatchClient(c, 0);
	}
	mpt_notify_fini(&no);
	
	return ret;
}

