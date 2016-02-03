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
	const char *fname;
	
	if (!ev) return 0;
	
	if (!solv) {
		MPT_ABORT("missing client descriptor");
	}
	src = 0;
	if (ev->msg) {
		MPT_STRUCT(msgtype) mt;
		MPT_STRUCT(message) msg;
		ssize_t part = -1;
		
		msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) == sizeof(mt)
		    && mt.cmd == MPT_ENUM(MessageCommand)
		    && (part = mpt_message_argv(&msg, mt.arg)) > 0) {
			part = mpt_message_read(&msg, part, 0);
			
			if (!(src = mpt_meta_message(&msg, mt.arg, '='))) {
				mpt_output_log(solv->out, __func__, MPT_FCNLOG(Error), "%s",
				               MPT_tr("failed to create argument stream"));
				return MPT_ERROR(BadOperation);
			}
			if ((fname = mpt_solver_read(solv, src))) {
				return MPT_event_fail(ev, fname);
			}
		}
	}
	/* problem config filename from configuration/terminal */
	if (!src) {
		MPT_INTERFACE(metatype) *cfg;
		cfg = solv->_vptr->cfg.query((void *) solv, 0);
		fname = cfg ? mpt_meta_data(cfg, 0) : 0;
		if (!fname || access(fname, R_OK) < 0) {
			static const char defName[] = "client.conf";
			char *rname, buf[1024];
			
			snprintf(buf, sizeof(buf), "%s [%s]: ", MPT_tr("problem settings"), defName);
			
			if (!(rname = mpt_readline(buf)) || !(fname = stripFilename(rname))) {
				fname = defName;
			}
			if (mpt_config_set((void *) solv, 0, fname, 0, 0) < 0) {
				return MPT_event_fail(ev, MPT_tr("unable to set client filename"));
			}
			free(rname);
		}
		else if ((fname = mpt_solver_read(solv, 0))) {
			return MPT_event_fail(ev, fname);
		}
		if (solv->out) mpt_output_log(solv->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("reading configuration file completed"));
	
		if (!mpt_config_get((void *) solv, "solconf", 0, 0)) {
			static const char defName[] = "solver.conf\0";
			char *rname, buf[1024];
			int ret;
			
			snprintf(buf, sizeof(buf), "%s [%s]: ", MPT_tr("solver parameter"), defName);
			
			if (!(rname = mpt_readline(buf)) || !(fname = stripFilename(rname))) {
				fname = defName;
			}
			ret = mpt_config_set((void *) solv, "solconf", fname, 0, 0);
			free(rname);
			
			if (ret < 0) {
				return MPT_event_fail(ev, MPT_tr("failed to set solver config"));
			}
			if (solv->out) mpt_output_log(solv->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("reading solver config completed"));
		}
	}
	/* initialize solver */
	if (solv->_vptr->init(solv, 0) < 0) {
		return MPT_event_fail(ev, MPT_tr("unable to initialize solver data"));
	}
	if (solv->out) mpt_output_log(solv->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("client initialisation finished"));
	
	/* prepare solver for run */
	if (solv->_vptr->prep(solv, 0) < 0) {
		return MPT_event_fail(ev, MPT_tr("solver preparation failed"));
	}
	if (solv->out) mpt_output_log(solv->out, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("client preparation finished"));
	
	/* configure default event to solver step */
	ev->id = mpt_hash("step", 4);
	ev->msg = 0;
	
	return MPT_event_cont(ev, MPT_tr("complete solver run"));
}

