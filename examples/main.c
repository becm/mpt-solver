/*!
 * main routine for MPT solver examples
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpt/client.h>
#include <mpt/array.h>
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
	struct mpt_notify *n;
	struct mpt_client *c;
	
	mtrace();
	
	/* create event handling */
	if (!(n = mpt_init(argc, argv))) {
		perror("mpt_init failed"); return 1;
	}
	/* setup problem type client */
	c = CREATE_CLIENT();
	
	/* setup event controller for client */
	if (mpt_client_events(n->_disp.arg, c) < 0) {
		perror("event setup failed"); return 2;
	}
	/* start event loop */
	mpt_loop(n);
	
	mpt_notify_fini(n);
	free(n);
	
	return 0;
}

