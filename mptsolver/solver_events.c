/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "array.h"
#include "message.h"
#include "event.h"
#include "output.h"

#include "client.h"
#include "meta.h"

#include "solver.h"

static int solevtRead(MPT_INTERFACE(client) *solv, MPT_STRUCT(event) *ev)
{
	if (!ev) return 0;
	
	if (!ev->msg) {
		if (mpt_config_set((void *) solv, 0, 0, 0, 0) < 0) {
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("no default configuration"));
		}
	}
	else {
		MPT_INTERFACE(metatype) *src;
		const char *cmd, *cfg, *sol;
		int ret;
		
		if (!(src = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
		cmd = cfg = sol = 0;
		/* consume command, client and solver config */
		if ((ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cmd)) < 0
		    || ret
		    || (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cfg)) < 0
		    || ret
		    || (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &sol)) < 0) {
			src->_vptr->unref(src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
		}
		if (ret && src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cmd)) {
			src->_vptr->unref(src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument count"));
		}
		if (cfg && mpt_config_set((void *) solv, 0, cfg, 0, 0) < 0) {
			src->_vptr->unref(src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("failed to read config"));
		}
		if (sol && mpt_config_set((void *) solv, "solver", cfg, 0, 0) < 0) {
			src->_vptr->unref(src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("failed to set client config"));
		}
		src->_vptr->unref(src);
	}
	return MPT_event_good(ev, MPT_tr("reading configuration files completed"));
}

static int solevtClear(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	
	if (!ev) return 0;
	if (!ev->msg) {
		/* clear single pass data */
		cl->_vptr->cfg.remove((void *) cl, 0);
		cl->_vptr->cfg.remove((void *) cl, &p);
	}
	else {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		MPT_INTERFACE(metatype) *src;
		const char *arg;
		int ret;
		
		if (!(src = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
		if ((ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &arg)) <= 0
		    || (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &arg)) < 0) {
			src->_vptr->unref(src);
			return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
		}
		/* clear single pass data */
		if (!ret) {
			cl->_vptr->cfg.remove((void *) cl, 0);
			cl->_vptr->cfg.remove((void *) cl, &p);
		}
		while (ret & MPT_ENUM(ValueConsume)) {
			if ((ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &arg)) < 0) {
				src->_vptr->unref(src);
				return MPT_event_fail(ev, MPT_ERROR(BadValue), MPT_tr("bad argument content"));
			}
			if (!ret) {
				break;
			}
			if (!arg) {
				continue;
			}
			mpt_path_set(&p, arg, -1);
			
			cl->_vptr->cfg.remove((void *) cl, &p);
		}
		src->_vptr->unref(src);
	}
	return MPT_event_stop(ev, MPT_tr("solver cleared"));
}

static const struct
{
	const char *name;
	int (*ctl)(MPT_INTERFACE(client) *, MPT_STRUCT(event) *);
}
cmdsolv[] = {
	{"read",    solevtRead },
	{"start",   mpt_solver_start },
	{"clear",   solevtClear }
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
				out->_vptr->obj.unref((void *) out);
			}
			i = sizeof(*dsp->_ctx) + sizeof(id);
			if (!dsp->_ctx && (dsp->_ctx = malloc(i))) {
				MPT_STRUCT(reply_context) *ctx = dsp->_ctx;
				ctx->ptr = dsp;
				ctx->len = 0;
				ctx->_max = sizeof(ctx->_val) + sizeof(id);
				ctx->used = 0;
			}
		}
	}
	/* register solver command handler */
	for (i = 0; i < MPT_arrsize(cmdsolv); i++) {
		MPT_STRUCT(command) *cmd;
		
		id = mpt_hash(cmdsolv[i].name, strlen(cmdsolv[i].name));
		
		if ((cmd = mpt_command_get(&dsp->_cmd, id))) {
			cmd->cmd = (int (*)()) cmdsolv[i].ctl;
			cmd->arg = cl;
			mpt_output_log(out, __func__, MPT_FCNLOG(Info), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("replaced handler id"), id, cmdsolv[i].name);
		}
		else if (mpt_dispatch_set(dsp, id, (int (*)()) cmdsolv[i].ctl, cl) < 0) {
			mpt_output_log(out, __func__, MPT_FCNLOG(Warning), "%s: %"PRIxPTR" (%s)\n",
				       MPT_tr("error registering handler id"), id, cmdsolv[i].name);
		}
	}
	
	return 0;
}
