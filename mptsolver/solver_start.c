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
#include "event.h"
#include "client.h"

#include "meta.h"

#include "solver.h"

static char *stripFilename(char *base)
{
	size_t len;
	
	while (isspace(*base)) ++base;
	if (!(len = strlen(base))) {
		return 0;
	}
	while (len-- && isspace(base[len])) {
		base[len] = 0;
	}
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
	int ret;
	
	if (!ev) return 0;
	
	/* running in remote-input mode */
	if (ev->msg) {
		if ((ret = mpt_solver_config((void *) solv, ev))) {
			return ret;
		}
	}
	/* problem config filename from configuration/terminal */
	else {
		static const char defExt[] = "conf\0";
		const MPT_INTERFACE(metatype) *cfg;
		const char *fname, *cname;
		char *rname, buf[128];
		
		/* check for existing config file */
		cfg = mpt_config_get((void *) solv, 0, 0, 0);
		fname = cfg ? mpt_meta_data(cfg, 0) : 0;
		
		cname = 0;
		if ((cfg = mpt_config_get(0, "mpt", 0, 0))
		    && cfg->_vptr->conv(cfg, 's', &cname) > 0
		    && cname) {
			const char *sep = strrchr(cname, '/');
			if (sep) cname = sep + 1;
		}
		if (!fname || access(fname, R_OK) < 0) {
			static const char defName[] = "client\0";
			const char *conf = cname ? cname : defName;
			snprintf(buf, sizeof(buf), "%s [%s.%s]: ",
			         MPT_tr("problem settings"), conf, defExt);
			
			if (!(rname = mpt_readline(buf))) {
				return MPT_event_fail(ev, MPT_ERROR(BadValue), "user interrupt");
			}
			if (!(fname = stripFilename(rname))) {
				snprintf(buf, sizeof(buf), "%s.%s", conf, defExt);
				fname = buf;
			}
			if (access(fname, R_OK) < 0) {
				mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s: %s",
				                  MPT_tr("file not readable"), fname);
				free(rname);
				ev->id = 0;
				return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
			}
			if (mpt_config_set((void *) solv, 0, fname, 0, 0) < 0) {
				mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s: %s",
				                  MPT_tr("failed to set client config"), fname);
				if (rname ) {
					free(rname);
				}
				ev->id = 0;
				return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
			}
			if (rname) {
				free(rname);
			}
		}
		/* config file has solver settings */
		cfg = mpt_config_get((void *) solv, "solconf", 0, 0);
		
		if (!cfg) {
			static const char defName[] = "solver\0", defPost[] = "sol\0";
			const char *sol = cname ? cname : defName;
			snprintf(buf, sizeof(buf), "%s [%s_%s.%s]: ",
			         MPT_tr("solver config"), sol, defPost, defExt);
			if (!(rname = mpt_readline(buf))) {
				return MPT_event_fail(ev, MPT_ERROR(BadValue), "user interrupt");
			}
			if (!(fname = stripFilename(rname))) {
				snprintf(buf, sizeof(buf), "%s_%s.%s", sol, defPost, defExt);
				fname = buf;
			}
			if (access(fname, R_OK) < 0) {
				mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s: %s",
				                  MPT_tr("file not readable"), fname);
				if (rname ) {
					free(rname);
				}
				ev->id = 0;
				return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
			}
			if ((ret = mpt_config_set((void *) solv, "solconf", fname, 0, 0)) < 0) {
				mpt_context_reply(ev->reply, MPT_ERROR(BadValue), "%s: %s",
				                  MPT_tr("failed to set solver config filename"), fname);
				if (rname ) {
					free(rname);
				}
				ev->id = 0;
				return MPT_ENUM(EventFail) | MPT_ENUM(EventDefault);
			}
			if (rname ) {
				free(rname);
			}
		}
	}
	/* initialize solver */
	ret = solv->_vptr->init(solv, 0);
	if (ret < 0) {
		return MPT_event_fail(ev, MPT_ERROR(BadValue), "failed to initialize client");
	}
	/* prepare solver for run */
	if ((ret = solv->_vptr->cfg.assign((void *) solv, 0, 0)) < 0) {
		return MPT_event_fail(ev, ret, MPT_tr("solver preparation failed"));
	}
	
	/* configure default event to solver step */
	ev->id = mpt_hash("step", 4);
	ev->msg = 0;
	
	return MPT_event_cont(ev, MPT_tr("complete solver run"));
}

