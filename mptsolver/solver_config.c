/*!
 * setup event controller for solver events
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#include <string.h>
#include <sys/uio.h>

#include "event.h"
#include "client.h"

#include "meta.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief configure solver
 * 
 * Set config files  for userdata and solver parameters.
 * 
 * \param solv config interface to solver client
 * \param ev   event with config message
 * 
 * \return event error code on failure
 */
extern int mpt_solver_config(MPT_INTERFACE(config) *solv, MPT_STRUCT(event) *ev)
{
	if (!ev) return 0;
	
	if (!ev->msg) {
		if (mpt_config_set(solv, 0, 0, 0, 0) < 0) {
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
		    || (ret && (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cfg)) < 0)
		    || (ret && (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &sol)) < 0)) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s (%d)", MPT_tr("bad argument type"), ret);
		}
		else if (ret && (ret = src->_vptr->conv(src, 's' | MPT_ENUM(ValueConsume), &cmd))) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s (%d)", MPT_tr("bad argument count"), ret);
			ret = -1;
		}
		else if ((ret = mpt_config_set(solv, 0, cfg, 0, 0)) < 0) {
			const char *err = MPT_tr("failed to read client config");
			if (cfg) {
				mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s: %s", err, cfg);
			} else {
				mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s", err);
			}
		}
		else if (sol && (ret = mpt_config_set(solv, "solver", sol, 0, 0)) < 0) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s: %s", MPT_tr("failed to read solver config"), sol);
		}
		src->_vptr->ref.unref((void *) src);
		if (ret < 0) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
	}
	return 0;
}
