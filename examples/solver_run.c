/*!
 * main routine for MPT solver examples
 */

#include <stdio.h>

#ifndef MPT_INCLUDE
# define MPT_INCLUDE(h) <mpt/h>
#endif

#include MPT_INCLUDE(event.h)
#include MPT_INCLUDE(client.h)

#include MPT_INCLUDE(notify.h)

#include MPT_INCLUDE(solver.h)

#include "solver_run.h"

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

static MPT_STRUCT(notify) no = MPT_NOTIFY_INIT;
static char * const *cfg = 0;

extern int client_init(int argc, char * const argv[])
{
	int pos;
	mtrace();
	
	/* environment and connection setup */
	if ((pos = mpt_init(&no, argc, argv)) < 0) {
		perror("mpt_init failed");
		return 1;
	}
	/* non-mpt arguments */
	if (pos < argc) {
		cfg = argv + pos;
	}
	return 0;
}

extern int solver_run(MPT_INTERFACE(client) *c)
{
	MPT_STRUCT(dispatch) disp = MPT_DISPATCH_INIT;
	MPT_STRUCT(value) val = MPT_VALUE_INIT;
	
	/* set client config root */
	val.ptr = "mpt.client";
	c->_vptr->cfg.assign((void *) c, 0, &val);
	
	/* setup dispatcher for solver client */
	if (mpt_solver_events(&disp, c) < 0) {
		perror("event setup failed");
		return 2;
	}
	/* set solver client arguments */
	if (cfg) {
		int take = mpt_solver_args((void *) c, cfg, -1);
		if (take < 0) {
			return 3;
		}
		cfg += take;
	}
	/* start event loop */
	mpt_notify_setdispatch(&no, &disp);
	mpt_loop(&no);
	
	mpt_notify_fini(&no);
	
	return 0;
}

