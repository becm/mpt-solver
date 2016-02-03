/*!
 * create client for IVP problem types
 */

#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include "node.h"
#include "config.h"
#include "parse.h"

#include "solver.h"

static int assignSolverConfig(MPT_STRUCT(node) *conf, const MPT_STRUCT(value) *val, MPT_INTERFACE(logger) *log)
{
	static const char defFmt[] = "[ ] = !#";
	static const char _func[] = "mpt_solver_assign";
	
	const char *fname, *format, *limit;
	FILE *fd;
	int ret;
	
	format = defFmt;
	limit = "ns";
	
	if (!val) {
		if (!(fname = mpt_node_data(conf, 0)) || !*fname) {
			return 0;
		}
	}
	else if (!val->fmt) {
		if (!(fname = val->ptr)) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s", MPT_tr("no solver config filename"));
			return MPT_ERROR(BadValue);
		}
	}
	else if (!val->ptr) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s", MPT_tr("no solver config data"));
		return MPT_ERROR(BadValue);
	}
	else if (val->fmt[0] == MPT_ENUM(TypeFile)) {
		FILE * const *ptr = val->ptr;
		
		fname = 0;
		if ((fd = *ptr)) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s", MPT_tr("no solver file descriptor"));
			return MPT_ERROR(BadValue);
		}
	}
	else if (val->fmt[0] == 's') {
		char * const *ptr = val->ptr;
		
		if (!(fname = *ptr) || !*fname) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s", MPT_tr("no solver file name"));
			return MPT_ERROR(BadValue);
		}
	}
	if (fname && !(fd = fopen(fname, "r"))) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s", MPT_tr("error opening solver config"), fname);
		return MPT_ERROR(BadValue);
	}
	ret = mpt_node_read(conf, fd, format, limit, log);
	
	if (fname) {
		if (ret < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s", MPT_tr("error parsing solver config"), fname);
		}
		fclose(fd);
	}
	return ret;
}

extern int mpt_solver_assign(MPT_STRUCT(node) *conf, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val, MPT_INTERFACE(logger) *log)
{
	MPT_INTERFACE(metatype) *m;
	MPT_STRUCT(value) tmp;
	const char *name;
	int ret;
	
	if (porg && porg->len) {
		MPT_STRUCT(path) p = *porg;
		MPT_STRUCT(node) *curr;
		ssize_t len = -1;
		
		if (val) {
			if (val->fmt || !val->ptr) {
				len = 0;
			} else {
				len = strlen(val->ptr) + 1;
			}
		}
		if (!(curr = mpt_node_query(conf->children, &p, len))) {
			if (!val) {
				return 0;
			}
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("failed to create config node"));
			return MPT_ERROR(BadOperation);
		}
		if (!conf->children) {
			conf->children = curr;
			curr->parent = conf;
		}
		if (p.len && !(curr = mpt_node_query(curr, &p, -1))) {
			if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s", MPT_tr("error while retrieving created node"));
			return MPT_ERROR(BadOperation);
		}
		p = *porg;
		name = p.base + p.off;
		len = mpt_path_next(&p);
		
		/* assign normal value */
		if (len == 7 && !strcmp(name, "solconf") && val) {
			return assignSolverConfig(curr, val, log);
		}
		if (!val) {
			return mpt_node_set(conf, 0);
		}
		if (!val->fmt) {
			return mpt_node_set(conf, val->ptr);
		}
		else if (val->fmt[0] == 's') {
			char * const *ptr = val->ptr;
			return mpt_node_set(conf, *ptr);
		}
		else if (curr->_meta) {
			return curr->_meta->_vptr->assign(curr->_meta, val);
		}
		return 0;
	}
	if (!val) {
		if (porg) {
			mpt_node_set(conf, 0);
			return 0;
		}
		/* get default config filename */
		else if (!(m = conf->_meta)
		         || m->_vptr->conv(m, 's', &name) < 0
		         || !name || !*name) {
			return MPT_ERROR(BadOperation);
		}
		/* parse existing top level config */
		tmp.fmt = 0;
		tmp.ptr = name;
		val = &tmp;
	}
	if ((ret = mpt_node_parse(conf, val, log)) < 0) {
		return ret;
	}
	/* set config file name */
	if (!val->fmt) {
		mpt_node_set(conf, val->ptr);
	}
	else if (val->fmt[0] == 's' && !val->fmt[1]) {
		char * const *ptr = val->ptr;
		mpt_node_set(conf, *ptr);
	}
	if (!(conf = mpt_node_find(conf, "solconf", 1))) {
		return ret;
	}
	if (assignSolverConfig(conf, 0, log) < 0) {
		return MPT_ERROR(BadArgument);
	}
	
	return ret;
}
