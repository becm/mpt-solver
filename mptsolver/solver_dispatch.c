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
extern int mpt_solver_dispatch(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(message) tmp;
	MPT_STRUCT(msgtype) mt;
	int ret;
	
	if (!ev->msg) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), "%s",
		                  MPT_tr("unable to dispatch default action"));
		ev->id = 0;
		return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
	}
	tmp = *ev->msg;
	
	if (!(ret = mpt_message_read(&tmp, sizeof(mt), &mt))) {
		mpt_context_reply(ev->reply, MPT_ERROR(MissingData), "%s",
		                  MPT_tr("empty message"));
		return MPT_ERROR(MissingData);
	}
	if (mt.cmd != MPT_MESGTYPE(Command)) {
		mpt_context_reply(ev->reply, MPT_ERROR(BadArgument), "%s (%02x != %02x)",
		                  MPT_tr("invalid message type"), mt.cmd, MPT_MESGTYPE(Command));
		return MPT_ERROR(BadArgument);
	}
	if (ret <= (int) sizeof(mt)) {
		ret = cl->_vptr->process(cl, mt.cmd, 0);
	} else {
		ret = mpt_client_command(cl, &tmp, mt.arg);
	}
	if (ret < 0) {
		mpt_context_reply(ev->reply, ret, "%s", MPT_tr("command processing failed"));
		ev->id = 0;
		ret = MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
	}
	if (ret & MPT_EVENTFLAG(Fail)) {
		ev->id = 0;
		ret |= MPT_EVENTFLAG(Default);
	}
	else if (ret & MPT_EVENTFLAG(Default)) {
		ev->id = MPT_MESGTYPE(Command);
	}
	if (ev->reply) {
		mpt_context_reply(ev->reply, 0, "%s", MPT_tr("command processing successful"));
	}
	return ret;
}
