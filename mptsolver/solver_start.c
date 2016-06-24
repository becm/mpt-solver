/*!
 * initialize and run solver (read, init, prep, cont).
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include "node.h"
#include "message.h"
#include "event.h"
#include "config.h"
#include "client.h"

#include "meta.h"

#include "solver.h"

static char *stripFilename(char *base)
{
	size_t len;
	
	while (isspace(*base)) ++base;
	if (!(len = strlen(base))) return 0;
	
	while (len-- && isspace(base[len]))
		base[len] = 0;
	
	return base;
}


/*!
 * \ingroup mptSolver
 * \brief client start operation
 * 
 * Take client and solver configuration files from event data.
 * Initialize and prepare client for run and display initial output.
 * 
 * \param solv  client descriptor
 * \param ev    event data
 * 
 * \return event registration hint or error
 */
extern int mpt_solver_start(MPT_INTERFACE(client) *solv, MPT_STRUCT(event) *ev)
{
	MPT_INTERFACE(metatype) *src;
	MPT_INTERFACE(logger) *log;
	const char *fname;
	int ret;
	
	if (!ev) return 0;
	
	if (!solv) {
		MPT_ABORT("missing client descriptor");
	}
	src = mpt_config_get((MPT_INTERFACE(config) *) solv, 0, 0, 0);
	log = 0;
	if (src) {
		src->_vptr->conv(src, MPT_ENUM(TypeLogger), &log);
	}
	src = 0;
	
	/* running in remot-input mode */
	if (ev->msg) {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(message) msg;
		ssize_t part;
		
		msg = *ev->msg;
		if ((part = mpt_message_read(&msg, sizeof(mt), &mt)) < (ssize_t) sizeof(mt)) {
			if (part) return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message type"));
			return MPT_event_fail(ev, MPT_ERROR(MissingData), MPT_tr("missing message header"));
		}
		if (mt.cmd != MPT_ENUM(MessageCommand)) {
			return MPT_event_fail(ev, MPT_ERROR(BadType), MPT_tr("bad message format"));
		}
		if ((part = mpt_message_argv(&msg, mt.arg)) > 0) {
			mpt_message_read(&msg, part, 0);
			if (mt.arg) mpt_message_read(&msg, 1, 0);
			part = mpt_message_argv(&msg, mt.arg);
		}
		if (part > 0 && !(src = mpt_meta_message(&msg, mt.arg))) {
			return MPT_event_fail(ev, MPT_ERROR(BadOperation), MPT_tr("failed to create argument stream"));
		}
		if ((fname = mpt_solver_read(solv, src))) {
			return MPT_event_fail(ev, MPT_ERROR(BadValue), fname);
		}
		if (src) src->_vptr->unref(src);
	}
	/* problem config filename from configuration/terminal */
	else {
		MPT_INTERFACE(metatype) *cfg;
		cfg = solv->_vptr->cfg.query((void *) solv, 0);
		fname = cfg ? mpt_meta_data(cfg, 0) : 0;
		if (!fname || access(fname, R_OK) < 0) {
			static const char defName[] = "client.conf";
			char *rname, buf[1024];
			int ret;
			
			snprintf(buf, sizeof(buf), "%s [%s]: ", MPT_tr("problem settings"), defName);
			
			if (!(rname = mpt_readline(buf)) || !(fname = stripFilename(rname))) {
				fname = defName;
			}
			if ((ret = mpt_config_set((void *) solv, 0, fname, 0, 0)) < 0) {
				return MPT_event_fail(ev, ret, MPT_tr("unable to set client file"));
			}
			free(rname);
		}
		else if ((fname = mpt_solver_read(solv, 0))) {
			return MPT_event_fail(ev, MPT_ERROR(BadValue), fname);
		}
		if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("reading configuration file completed"));
	
		if (!mpt_config_get((void *) solv, "solconf", 0, 0)) {
			static const char defName[] = "solver.conf\0";
			char *rname, buf[1024];
			
			snprintf(buf, sizeof(buf), "%s [%s]: ", MPT_tr("solver parameter"), defName);
			
			if (!(rname = mpt_readline(buf)) || !(fname = stripFilename(rname))) {
				fname = defName;
			}
			ret = mpt_config_set((void *) solv, "solconf", fname, 0, 0);
			free(rname);
			
			if (ret < 0) {
				return MPT_event_fail(ev, ret, MPT_tr("failed to set solver config"));
			}
			if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("reading solver config completed"));
		}
	}
	/* initialize solver */
	if ((ret = solv->_vptr->init(solv, 0)) < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("unable to initialize solver data"));
	}
	if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("client initialisation finished"));
	
	/* prepare solver for run */
	if ((ret = solv->_vptr->prep(solv, 0)) < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("solver preparation failed"));
	}
	if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("client preparation finished"));
	
	/* configure default event to solver step */
	ev->id = mpt_hash("step", 4);
	ev->msg = 0;
	
	return MPT_event_cont(ev, MPT_tr("complete solver run"));
}

