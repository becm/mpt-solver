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

static int parseNode(MPT_STRUCT(node) *conf, const char *fname, MPT_INTERFACE(iterator) *args, const char *format, MPT_INTERFACE(logger) *log)
{
	static const char _func[] = "mpt_solver_read";
	MPT_STRUCT(value) val;
	const char *dat[2];
	int ret;
	
	/* use fallback name */
	if (!args) {
		if (!(val.ptr = fname) || !*fname) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("no default name"));
			return MPT_ERROR(BadValue);
		}
		val.fmt = 0;
	}
	else if ((ret = args->_vptr->get(args, MPT_ENUM(TypeValue), &val)) < 0) {
		if ((ret = args->_vptr->get(args, 's', &fname)) < 0
		 || !fname) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("no solver config filename"));
			return MPT_ERROR(BadValue);
		}
		val.fmt = "ss";
		val.ptr = dat;
		
		dat[0] = fname;
		dat[1] = format;
	}
	return mpt_node_parse(conf, &val, log);
}

extern int mpt_solver_read(MPT_STRUCT(node) *conf, MPT_STRUCT(iterator) *args, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) *sol, cfg = MPT_NODE_INIT;
	int ret, err;
	
	/* parse top level config */
	if ((err = parseNode(&cfg, mpt_node_data(conf, 0), args, "{*} = !#", log)) < 0) {
		return err;
	}
	ret = err ? 1 : 0;

	if ((sol = cfg.children)
	    && !(sol = mpt_node_next(sol, "solconf"))) {
		if (!(sol = mpt_node_new(8, 0))) {
			if (log) mpt_log(log, __func__, MPT_LOG(Critical), "%s", MPT_tr("failed to create solver config node"));
			mpt_node_clear(&cfg);
			return MPT_ERROR(BadOperation);
		}
		mpt_gnode_insert(&cfg, 0, sol);
	}
	/* require successful advance */
	if (args && args->_vptr->advance(args) < 0) {
		args = 0;
	}
	/* parse solver config file */
	if ((err = parseNode(sol, mpt_node_data(sol, 0), args, "[ ] = !#", log)) < 0) {
		mpt_node_clear(&cfg);
		return err;
	}
	if (err) {
		++ret;
	}
	/* check further arguments */
	if (args && args->_vptr->advance(args) > 0) {
		if (log) mpt_log(log, __func__, MPT_LOG(Critical), "%s", MPT_tr("too many arguments"));
		mpt_node_clear(&cfg);
		return MPT_ERROR(BadArgument);
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
