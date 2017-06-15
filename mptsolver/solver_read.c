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

static int parseNode(MPT_STRUCT(node) *conf, const char *fname, const MPT_INTERFACE(metatype) *args, const char *format, const char *limit, MPT_INTERFACE(logger) *log)
{
	static const char _func[] = "mpt_solver_read";
	MPT_STRUCT(value) val;
	FILE *fd = 0;
	int ret;
	
	/* use fallback name */
	if (!args) {
		if (fname || !*fname) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("no default name"));
			return MPT_ERROR(BadValue);
		}
	}
	else if ((ret = args->_vptr->conv(args, MPT_ENUM(TypeValue), &val)) > 0) {
		const char *tmp;
		void * const *ptr = val.ptr;
		
		if (!val.fmt) {
			if (!(fname = val.ptr)) {
				if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("no solver config filename"));
				return MPT_ERROR(BadValue);
			}
		}
		else if (val.fmt[0] == MPT_ENUM(TypeFile)) {
			if (!ptr || !(fd = ptr[0])) {
				if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad file argument"));
				return MPT_ERROR(BadValue);
			}
			fname = 0;
		}
		else if (val.fmt[0] == 's') {
			if (!ptr || !(fname = ptr[0])) {
				if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad filename argument"));
				return MPT_ERROR(BadValue);
			}
		}
		else {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad file type argument"));
			return MPT_ERROR(BadValue);
		}
		if (val.fmt[1] == 's') {
			if ((tmp = ptr[1])) {
				format = tmp;
			}
			if (val.fmt[2] == 's') {
				if ((tmp = ptr[2])) {
					limit = tmp;
				}
			}
			else if (val.fmt[2]) {
				if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad file limit argument"));
				return MPT_ERROR(BadType);
			}
		}
		else if (val.fmt[1]) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad file format argument"));
			return MPT_ERROR(BadType);
		}
	}
	else if (!ret) {
		if (!fname) {
			if ((fname = mpt_node_ident(conf))) {
				if (log) mpt_log(log, _func, MPT_LOG(Debug), "%s: %s", fname, MPT_tr("no default filename"));
			} else {
				if (log) mpt_log(log, _func, MPT_LOG(Debug), "%s", MPT_tr("no default filename"));
			}
			return MPT_ERROR(BadValue);
		}
	}
	else if ((ret = args->_vptr->conv(args, MPT_ENUM(TypeFile), &fd)) > 0) {
		if (!fd) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad client config file descriptor"));
			return MPT_ERROR(BadValue);
		}
		fname = 0;
	}
	else if ((ret = args->_vptr->conv(args, 's', &fname)) > 0) {
		if (!fname || !*fname) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s", MPT_tr("bad client config file name"));
			return MPT_ERROR(BadValue);
		}
	}
	
	if (fname && !(fd = fopen(fname, "r"))) {
		if (log) mpt_log(log, _func, MPT_LOG(Error), "%s: %s", MPT_tr("error opening solver config"), fname);
		return MPT_ERROR(BadValue);
	}
	ret = mpt_node_read(conf, fd, format, limit, log);
	
	if (fname) {
		if (ret < 0) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s: %s", MPT_tr("error parsing solver config"), fname);
		}
		fclose(fd);
	}
	return ret;
}

extern int mpt_solver_read(MPT_STRUCT(node) *conf, MPT_STRUCT(iterator) *args, MPT_INTERFACE(logger) *log)
{
	MPT_STRUCT(node) *sol, cfg = MPT_NODE_INIT;
	int ret, err;
	
	/* parse top level config */
	if ((err = parseNode(&cfg, mpt_node_data(conf, 0), (void *) args, "{*} = !#", "ns", log)) < 0) {
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
	if ((err = parseNode(sol, mpt_node_data(sol, 0), (void *) args, "[ ] = !#", "ns", log)) < 0) {
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
