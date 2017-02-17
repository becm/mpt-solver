/*!
 * main routine for MPT solver examples
 */

#include <stdio.h>

#include <mpt/event.h>
#include <mpt/client.h>

#include <mpt/notify.h>

#include <mpt/solver.h>

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

static struct mpt_notify no = MPT_NOTIFY_INIT;

extern int client_init(int argc, char *argv[])
{
	mtrace();
	
	/* create event handling */
	if (mpt_init(&no, argc, argv) < 0) {
		perror("mpt_init failed");
		return 1;
	}
	return 0;
}

extern int solver_run(struct mpt_client *c)
{
	struct mpt_dispatch disp = MPT_DISPATCH_INIT;
	
	/* setup dispatcher for solver client */
	if (mpt_solver_events(&disp, c) < 0) {
		perror("event setup failed");
		return 2;
	}
	/* start event loop */
	mpt_notify_setdispatch(&no, &disp);
	mpt_loop(&no);
	
	mpt_notify_fini(&no);
	
	return 0;
}

