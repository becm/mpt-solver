/*!
 * read basic and specific configuration from files.
 */

#include <string.h>
#include <errno.h>

#include "node.h"
#include "message.h"
#include "config.h"

#include "client.h"

/*!
 * \ingroup mptSolver
 * \brief read client configuration
 * 
 * Try to get client and solver configuration files from message,
 * fall back to values in client configuration and read files.
 * 
 * \param conf configuration target
 * \param msg  message data
 * \param sep  message argument separator
 * \param log  logging descriptor
 * 
 * \return string describing error
 */
extern const char *mpt_solver_read(MPT_STRUCT(node) *conf, MPT_STRUCT(message) *msg, int sep, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) *tmp;
	const char *fname, *format;
	char fbuf[1024];
	ssize_t part = -1;
	
	if ((fname = mpt_client_read(conf, msg, sep, log))) {
		return fname;
	}
	/* set format for solver configuration file */
	tmp = mpt_node_next(conf->children, "solconf_fmt");
	
	if (!tmp || !(format = mpt_node_data(tmp, 0))) {
		format = "[ ] =\n#!";
	}
	tmp = mpt_node_next(conf->children, "solconf");
	
	/* use second argument as solver configuration */
	if (!msg || (part = mpt_message_argv(msg, sep)) <= 0) {
		if (!tmp || !(fname = mpt_node_data(tmp, 0))) {
			return 0;
		}
	} else if ((size_t) part < msg->used && (!sep || !((char *) msg->base)[part])) {
		fname = msg->base;
		part = mpt_message_read(msg, part, 0);
	} else if (part >= (ssize_t) sizeof(fbuf)) {
		return MPT_tr("temporary buffer exceeded");
	} else {
		part = mpt_message_read(msg, part, fbuf);
		fbuf[part] = '\0';
		fname = fbuf;
	}
	/* save to configuration if unset */
	if (!tmp) {
		if (!(tmp = mpt_node_new(part+1, "solconf", -7))) {
			return MPT_tr("node create error");
		}
		if (mpt_gnode_insert(conf, 0, tmp) < 0) {
			mpt_node_destroy(tmp);
			return MPT_tr("node insert error");
		}
		mpt_node_set(tmp, fname);
	}
	/* read solver configuration */
	if (mpt_config_read(tmp, fname, format, 0, log) < 0) {
		return MPT_tr("error in solver configuration");
	}
	return 0;
}

