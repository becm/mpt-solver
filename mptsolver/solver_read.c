/*!
 * create client for IVP problem types
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include "node.h"
#include "client.h"
#include "config.h"
#include "parse.h"

#include "meta.h"

#include "solver.h"

static const char solconfName[] = "solconf";

static int parseNode(MPT_STRUCT(node) *conf, MPT_INTERFACE(iterator) *args, const char *fname, const char *format, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(value) val;
	const char *dat[2];
	int ret;
	
	/* use fallback name */
	val.fmt = "ss";
	val.ptr = dat;
	
	dat[0] = fname;
	dat[1] = format;
	
	if (args && (ret = args->_vptr->get(args, MPT_ENUM(TypeValue), &val)) < 0) {
		if ((ret = args->_vptr->get(args, 's', &fname)) < 0
		    || !fname) {
			ret = args->_vptr->get(args, 0, 0);
			mpt_log(log, __func__, MPT_LOG(Error), "%s: %d",
			        MPT_tr("invalid config type"), ret);
			return MPT_ERROR(BadValue);
		}
	}
	return mpt_node_parse(conf, &val, log);
}

static int parseSolconf(MPT_STRUCT(node) *conf, MPT_STRUCT(iterator) *args, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) *sol;
	int err;
	
	if (!(sol = conf->children)
	    || !(sol = mpt_node_next(sol, solconfName))) {
		/* create new config node */
		if (!(sol = mpt_node_new(8))) {
			if (log) {
				mpt_log(log, __func__, MPT_LOG(Critical), "%s",
				        MPT_tr("failed to create solver config node"));
			}
			return MPT_ERROR(BadOperation);
		}
		/* name and insert new node */
		mpt_identifier_set(&sol->ident, solconfName, -1);
		mpt_gnode_insert(conf, 0, sol);
	}
	/* parse solver config */
	if ((err = parseNode(sol, args, 0, "[ ] = !#", log)) < 0) {
		return err;
	}
	/* check further arguments */
	if (args->_vptr->advance(args) > 0) {
		if (log) {
		        mpt_log(log, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("too many arguments"));
		}
		return MPT_ERROR(BadArgument);
	}
	return 0;
}

extern int mpt_solver_read(MPT_STRUCT(node) *conf, MPT_STRUCT(iterator) *args, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) *sol, cfg = MPT_NODE_INIT;
	const char *fname;
	int ret = 1, err;
	
	/* parse top level config */
	fname = mpt_node_data(conf, 0);
	if (args) {
		if ((err = parseNode(&cfg, args, fname, "{*} = !#", log)) < 0) {
			return err;
		}
		ret = 3;
		if (args->_vptr->advance(args) <= 0) {
			if ((sol = cfg.children)
			    && (sol = mpt_node_next(sol, solconfName))) {
				fname = mpt_node_data(sol, 0);
			}
			ret = 1;
		}
		else if ((err = parseSolconf(&cfg, args, log)) < 0) {
			mpt_node_clear(&cfg);
			return err;
		}
	}
	/* keep existing top config */
	else if ((sol = conf->children) || !fname) {
		if (!(sol = mpt_node_next(sol, solconfName))
		    || !(fname = mpt_node_data(sol, 0))) {
			return 0;
		}
		if ((err = parseNode(sol, 0, fname, "[ ] = !#", log)) < 0) {
			return err;
		}
		return 2;
	}
	/* read new top level config */
	else if ((err = parseNode(&cfg, 0, fname, "{*} = !#", log)) < 0) {
		mpt_log(log, __func__, MPT_LOG(Error), "%s",
		        MPT_tr("failed to read client config"));
		return err;
	}
	/* config filename in new client config */
	else if ((sol = cfg.children)
	    && (sol = mpt_node_next(sol, solconfName))) {
		fname = mpt_node_data(sol, 0);
	}
	/* config has filename */
	if (fname) {
		if ((err = parseNode(sol, 0, fname, "[ ] = !#", log)) < 0) {
			mpt_node_clear(&cfg);
			return err;
		}
		ret = 3;
	}
	/* replace solver configuration */
	mpt_node_clear(conf);
	conf->children = sol = cfg.children;
	while (sol) {
		sol->parent = conf;
		sol = sol->next;
	}
	return ret;
}
