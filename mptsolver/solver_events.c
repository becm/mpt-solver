/*!
 * setup event controller for solver events
 */

#include <inttypes.h>

#include <string.h>

#include "array.h"
#include "message.h"
#include "event.h"
#include "output.h"
#include "client.h"

#include "meta.h"

#include "solver.h"

static int solevtRead(MPT_INTERFACE(client) *solv, MPT_STRUCT(event) *ev)
{
	int err;
	
	if (!ev) return 0;
	
	if ((err = mpt_solver_config((void *) solv, ev))) {
		return err;
	}
	return MPT_event_good(ev, MPT_tr("reading configuration files completed"));
}
static int solevtClear(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	return mpt_cevent_clear((void *) cl, ev);
}
static int solevtPrepare(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	int err;
	if (!ev) {
		return 0;
	}
	if (ev->msg) {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(message) msg = *ev->msg;
		ssize_t part;
		
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
	}
	if ((err = cl->_vptr->cfg.assign((void *) cl, 0, 0)) < 0) {
		return MPT_event_fail(ev, err, MPT_tr("failed to prepare slver"));
	}
	return MPT_event_good(ev, MPT_tr("solver client configured"));
}

static const struct
{
	const char *name;
	int (*ctl)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
}
cmdsolv[] = {
	{"start",  mpt_solver_start },
	{"prep",   solevtPrepare },
	{"clear",  solevtClear },
	{"read",   solevtRead }
};

/*!
 * \ingroup mptSolver
 * \brief register client events
 * 
 * Set event handlers for solver client in dispatch descriptor.
 * Try to register dispatch output as solver client output
 * and dispatch graphic commands to output descriptor.
 * 
 * \param dsp dispatch descriptor
 * \param cl  client descriptor
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_solver_events(MPT_STRUCT(dispatch) *dsp, MPT_INTERFACE(client) *cl)
{
	MPT_INTERFACE(output) *out;
	uintptr_t id;
	size_t i;
	int err;
	
	if ((err = mpt_client_events(dsp, cl)) < 0) {
		return err;
	}
	if ((out = dsp->_err.arg)) {
		static const char fmt[2] = { MPT_ENUM(TypeOutput) };
		MPT_STRUCT(value) val;
		val.fmt = fmt;
		val.ptr = &out;
		
		/* assign output to client */
		if (cl->_vptr->cfg.assign((void *) cl, 0, &val) >= 0
		    && out->_vptr->obj.addref((void *) out)) {
			if (mpt_dispatch_control(dsp, "graphic", out) < 0) {
				out->_vptr->obj.ref.unref((void *) out);
			}
		}
	}
	/* register solver command handler */
	for (i = 0; i < MPT_arrsize(cmdsolv); i++) {
		MPT_STRUCT(command) *cmd;
		
		id = mpt_hash(cmdsolv[i].name, strlen(cmdsolv[i].name));
		
		if ((cmd = mpt_command_get(&dsp->_d, id))) {
			cmd->cmd = (int (*)()) cmdsolv[i].ctl;
			cmd->arg = cl;
			mpt_output_log(out, __func__, MPT_LOG(Info), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("replaced handler id"), id, cmdsolv[i].name);
		}
		else if (mpt_dispatch_set(dsp, id, (int (*)()) cmdsolv[i].ctl, cl) < 0) {
			mpt_output_log(out, __func__, MPT_LOG(Warning), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("error registering handler id"), id, cmdsolv[i].name);
		}
	}
	return 0;
}
