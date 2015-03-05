/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "array.h"
#include "message.h"
#include "event.h"

#include "client.h"

#include "solver.h"

static int solevtRead(MPT_INTERFACE(client) *solv, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_INTERFACE(logger) *log;
	const char *err;
	
	if (!ev) return 0;
	
	if (!solv->conf && !(solv->conf = mpt_client_config("client"))) {
		return MPT_event_fail(ev, MPT_tr("unable to query configuration"));
	}
	log = MPT_LOGGER((MPT_INTERFACE(metatype) *) solv->out);
	
	if (!ev->msg) {
		err = mpt_client_read(solv->conf, 0, 0, log);
	}
	else {
		MPT_STRUCT(message) msg = *ev->msg;
		size_t part;
		
		if (mpt_message_read(&msg, sizeof(mt), &mt) < sizeof(mt)) {
			return MPT_event_fail(ev, MPT_tr("missing message type"));
		}
		/* consume command part */
		if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			mpt_message_read(&msg, part, 0);
		}
		err = mpt_client_read(solv->conf, &msg, mt.arg, log);
	}
	if (err) {
		return MPT_event_fail(ev, err);
	}
	return MPT_event_good(ev, MPT_tr("reading configuration files completed"));
}

static const struct
{
	const char *name;
	int (*ctl)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
}
cmdsolv[] = {
	{"read",    solevtRead      },
	{"start",   mpt_solver_start}
};

/*!
 * \ingroup mptSolver
 * \brief register client events
 * 
 * Set event handlers in dispatch descriptor.
 * 
 * \param dsp dispatch descriptor
 * \param cl  client descriptor
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_solver_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(client) *cl)
{
	uintptr_t id;
	size_t i;
	int err;
	
	if ((err = mpt_client_events(dsp, cl)) < 0) {
		return err;
	}
	/* register solver command handler */
	for (i = 0; i < MPT_arrsize(cmdsolv); i++) {
		MPT_STRUCT(command) *cmd;
		
		id = mpt_hash(cmdsolv[i].name, strlen(cmdsolv[i].name));
		
		if ((cmd = mpt_command_get(&dsp->_cmd, id))) {
			cmd->cmd = (int (*)()) cmdsolv[i].ctl;
			cmd->arg = cl;
			mpt_output_log(dsp->_out, __func__, MPT_ENUM(LogInfo), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("replaced handler id"), id, cmdsolv[i].name);
		}
		else if (mpt_dispatch_set(dsp, id, (int (*)()) cmdsolv[i].ctl, cl) < 0) {
			mpt_output_log(dsp->_out, __func__, MPT_ENUM(LogWarning), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("error registering handler id"), id, cmdsolv[i].name);
		}
	}
	
	return 0;
}
