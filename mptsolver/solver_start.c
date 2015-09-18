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
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(msgtype) mt = MPT_MSGTYPE_INIT;
	MPT_STRUCT(message) msg = MPT_MESSAGE_INIT;
	ssize_t part = -1;
	const char *fname;
	
	if (!ev) return 0;
	
	if (ev->msg) {
		msg = *ev->msg;
		if (mpt_message_read(&msg, sizeof(mt), &mt) == sizeof(mt)
		    && mt.cmd == MPT_ENUM(MessageCommand)
		    && (part = mpt_message_argv(&msg, mt.arg)) > 0) {
			part = mpt_message_read(&msg, part, 0);
		}
	}
	if (!solv) {
		MPT_ABORT("missing client descriptor");
	}
	solv->_vptr->clear(solv);
	
	if (!solv->conf && !(solv->conf = mpt_client_config("client"))) {
		return MPT_event_fail(ev, MPT_tr("unable to query configuration"));
	}
	/* problem config filename from arguments/configuration/terminal */
	if (part < 0 || mpt_message_argv(&msg, mt.arg) <= 0) {
		fname = solv->conf ? (char *) mpt_node_data(solv->conf, 0) : 0;
		if (!fname || access(fname, R_OK) < 0) {
			static const char defName[] = "client.conf";
			char *rname, buf[1024];
			
			snprintf(buf, sizeof(buf), "%s [%s]: ", MPT_tr("problem settings"), defName);
			
			if (!(rname = mpt_readline(buf)) || !(fname = stripFilename(rname))) {
				fname = defName;
			}
			if (mpt_node_set(solv->conf, fname) < 0) {
				return MPT_event_fail(ev, MPT_tr("unable to set client filename"));
			}
			free(rname);
		}
	}
	log = MPT_LOGGER((MPT_INTERFACE(metatype) *) solv->out);
	
	/* read configuration files */
	if ((fname = mpt_solver_read(solv->conf, &msg, mt.arg, log))) {
		return MPT_event_fail(ev, fname);
	}
	if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("reading configuration files completed"));
	
	/* solver config filename from terminal */
	if (!mpt_node_next(solv->conf->children, "solconf")) {
		MPT_STRUCT(node) *conf;
		static const char defName[] = "solver.conf", sc[] = "solconf";
		const char *format;
		char *rname, buf[1024];
		
		snprintf(buf, sizeof(buf), "%s [%s]: ", MPT_tr("solver parameter"), defName);
		
		if (!(rname = mpt_readline(buf)) || !(fname = stripFilename(rname))) {
			fname = defName;
		}
		/* set format for solver configuration file */
		conf = mpt_node_next(solv->conf->children, "solconf_fmt");
		format = conf ? mpt_node_data(conf, 0) : "[ ] =\n#!";
		
		if (!(conf = mpt_node_new(sizeof(sc), strlen(fname) + 1))) {
			free(rname);
			return MPT_event_fail(ev, MPT_tr("unable to create solver configuration"));
		}
		mpt_identifier_set(&conf->ident, "solconf", strlen(sc));
		if (mpt_node_set(conf, fname) < 0) {
			free(rname);
			mpt_node_destroy(conf);
			return MPT_event_fail(ev, MPT_tr("unable to set solver filename"));
		}
		if (mpt_config_read(conf, fname, format, 0, log) < 0) {
			free(rname);
			mpt_node_destroy(conf);
			return MPT_event_fail(ev, "error in solver configuration");
		}
		mpt_gnode_insert(solv->conf, 0, conf);
		free(rname);
	}
	
	/* initialize solver */
	if (solv->_vptr->init(solv) < 0) {
		return MPT_event_fail(ev, MPT_tr("unable to initialize solver data"));
	}
	if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("client initialisation finished"));
	
	/* prepare solver for run */
	if (solv->_vptr->prep(solv, 0) < 0) {
		return MPT_event_fail(ev, MPT_tr("solver preparation failed"));
	}
	if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("client preparation finished"));
	
	if (solv->_vptr->output(solv, MPT_ENUM(OutputStateInit)) < 0) {
		return MPT_event_fail(ev, MPT_tr("initial output failed"));
	}
	if (log) mpt_log(log, __func__, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("initial output pushed"));
	
	/* configure default event to solver step */
	ev->id = mpt_hash("step", 4);
	ev->msg = 0;
	
	return MPT_event_cont(ev, MPT_tr("complete solver run"));
}

