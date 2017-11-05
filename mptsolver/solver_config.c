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

#include "config.h"
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
		MPT_INTERFACE(iterator) *args;
		const char *cmd, *cfg, *sol;
		int ret;
		
		if (!(src = mpt_event_command(ev))) {
			ev->id = 0;
			return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
		}
		args = 0;
		cmd = cfg = sol = 0;
		if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &args)) < 0
		    || !args) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s (%d)", MPT_tr("bad argument source"), ret);
		}
		else if ((ret = src->_vptr->conv(src, 's', &cmd)) < 0) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s (%d)", MPT_tr("bad command type"), ret);
		}
		/* get client and solver config */
		else if ((ret = args->_vptr->get(args, 's', &cfg)) < 0
		         || ((ret = args->_vptr->advance(args)) > 0
		             && (ret = args->_vptr->get(args, 's', &sol)) < 0)) {
			mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s (%d)", MPT_tr("bad argument type"), ret);
		}
		else if ((ret = args->_vptr->advance(args)) > 0) {
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
