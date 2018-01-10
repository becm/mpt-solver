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

#include "config.h"
#include "meta.h"

#include "solver.h"

static MPT_INTERFACE(config) *getConfig(MPT_INTERFACE(client) *cl, MPT_INTERFACE(reply_context) *rc)
{
	MPT_INTERFACE(config) *cfg = 0;
	if (cl->_vptr->meta.conv((void *) cl, MPT_ENUM(TypeConfig), &cfg) > 0
	    && cfg) {
		return cfg;
	}
	mpt_context_reply(rc, MPT_ERROR(BadType), "%s",
	                  MPT_tr("client without config interface"));
	return cfg;
}

static int setConf(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(config) *cfg = ptr;
	return cfg->_vptr->assign(cfg, p, val);
}
static int condConf(void *ptr, const MPT_STRUCT(path) *p, const MPT_STRUCT(value) *val)
{
	MPT_INTERFACE(config) *cfg = ptr;
	int ret;
	if (cfg->_vptr->query(cfg, p)) {
		return 0;
	}
	ret = cfg->_vptr->assign(cfg, p, val);
	return ret ? ret : 1;
}
/*!
 * \ingroup mptSolver
 * \brief process command event
 * 
 * Call client command process function with
 * content in event message data.
 * Set command as default event type if client requests
 * default operation.
 * 
 * \param dsp dispatch descriptor
 * \param ev  client descriptor
 * 
 * \retval 0  success
 * \retval -1 missing descriptor pointer
 */
extern int mpt_solver_dispatch(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(message) tmp;
	MPT_STRUCT(msgtype) mt;
	int ret;
	
	if (!ev) {
		return 0;
	}
	if (!ev->msg) {
		return MPT_event_fail(ev, MPT_ERROR(BadArgument), MPT_tr("no default action"));
	}
	tmp = *ev->msg;
	
	/* get message header */
	mt.arg = 0;
	if (!(ret = mpt_message_read(&tmp, sizeof(mt), &mt))) {
		return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("empty message"));
	}
	/* process parameter data */
	if (mt.cmd == MPT_MESGTYPE(ParamGet)) {
		MPT_INTERFACE(config) *cfg;
		if (!(cfg = getConfig(cl, ev->reply))) {
			return MPT_EVENTFLAG(Fail);
		}
		ret = mpt_config_reply(ev->reply, cfg, mt.arg, &tmp);
		if (ret < 0) {
			return MPT_event_fail(ev, ret, MPT_tr("parameter collection failed"));
		}
		return MPT_EVENTFLAG(None);
	}
	if (mt.cmd == MPT_MESGTYPE(ParamSet)) {
		MPT_INTERFACE(config) *cfg;
		if (!(cfg = getConfig(cl, ev->reply))) {
			return MPT_EVENTFLAG(Fail);
		}
		ret = mpt_message_assign(&tmp, mt.arg, setConf, cfg);
		if (ret < 0) {
			return MPT_event_fail(ev, ret, MPT_tr("parameter assignment failed"));
		}
		return MPT_event_good(ev, MPT_tr("parameter assigned"));
	}
	if (mt.cmd == MPT_MESGTYPE(ParamCond)) {
		MPT_INTERFACE(config) *cfg;
		if (!(cfg = getConfig(cl, ev->reply))) {
			return MPT_EVENTFLAG(Fail);
		}
		ret = mpt_message_assign(&tmp, mt.arg, condConf, cfg);
		if (ret < 0) {
			return MPT_event_fail(ev, ret, MPT_tr("parameter assignment failed"));
		}
		if (!ret) {
			return MPT_event_good(ev, MPT_tr("parameter unchanged"));
		} else {
			return MPT_event_good(ev, MPT_tr("parameter assigned"));
		}
	}
	/* require command message */
	if (mt.cmd != MPT_MESGTYPE(Command)) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), "%s (%02x != %02x)",
		                  MPT_tr("invalid message type"), mt.cmd, MPT_MESGTYPE(Command));
		return MPT_EVENTFLAG(Fail);
	}
	if (ret < (int) sizeof(mt)) {
		ret = cl->_vptr->process(cl, mt.cmd, 0);
	} else {
		ret = mpt_client_command(cl, &tmp, mt.arg);
	}
	if (ret < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("command processing failed"));
	}
	if (ret & MPT_EVENTFLAG(Fail)) {
		if (ret & MPT_EVENTFLAG(Default)) {
			ev->id = 0;
		}
		mpt_context_reply(ev->reply, MPT_ERROR(BadOperation), "%s", MPT_tr("command processing failed"));
	}
	else if (ret & MPT_EVENTFLAG(Default)) {
		ev->id = MPT_MESGTYPE(Command);
		mpt_context_reply(ev->reply, 0, "%s", MPT_tr("set default command"));
	}
	else {
		mpt_context_reply(ev->reply, 0, "%s", MPT_tr("command processing successful"));
	}
	return ret;
}
