/*!
 * main routine for MPT solver examples
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpt/event.h>
#include <mpt/notify.h>

#ifndef CREATE_CLIENT
# error: client creator undefined
#endif

#ifdef __GLIBC__
# include <mcheck.h>
#else
# define mtrace()
#endif

extern int main(int argc, char *argv[])
{
	struct mpt_notify no = MPT_NOTIFY_INIT;
	struct mpt_dispatch disp = MPT_DISPATCH_INIT;
	struct mpt_client *c;
	
	mtrace();
	
	/* create event handling */
	if (mpt_init(&no, argc, argv) < 0) {
		perror("mpt_init failed");
		return 1;
	}
	/* setup problem type client */
	c = CREATE_CLIENT("mpt.client");
	
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

