/*!
 * MPT solver library
 *   read config files
 */

#include <string.h>
#include <stdio.h>

#include "convert.h"
#include "meta.h"
#include "node.h"
#include "parse.h"
#include "output.h"

#include "solver.h"


static int getArg(MPT_INTERFACE(iterator) *args, FILE **file, const char **fname, MPT_INTERFACE(logger) *log, const char *_func)
{
	const MPT_STRUCT(value) *val;
	int ret;
	/* get client config file parameters from argument */
	if (!(val = args->_vptr->value(args))) {
		if (log) {
			mpt_log(log, _func, MPT_LOG(Error), "%s",
			        MPT_tr("invalid file pointer"));
		}
		return MPT_ERROR(MissingData);
	}
	if ((ret = mpt_value_convert(val, MPT_ENUM(TypeFilePtr), file)) > 0) {
		if (*file) {
			if ((ret = args->_vptr->advance(args)) < 0) {
				return ret;
			}
			return MPT_ENUM(TypeFilePtr);
		}
		if (log) {
			mpt_log(log, _func, MPT_LOG(Error), "%s",
			        MPT_tr("invalid file pointer"));
		}
		return MPT_ERROR(MissingData);
	}
	if ((ret = mpt_value_convert(val, 's', fname)) < 0) {
		const MPT_STRUCT(value) *val = args->_vptr->value(args);
		if (log) {
			int type = args ? val->_type : 0;
			int namespace = args ? val->_namespace : 0;
			mpt_log(log, _func, MPT_LOG(Error), "%s (%d@$d)",
			        MPT_tr("invalid file argument type"), type, namespace);
			
		}
		return ret;
	}
	if (!*fname) {
		if (log) {
			mpt_log(log, _func, MPT_LOG(Error), "%s",
			        MPT_tr("missing file name"));
			
		}
		return MPT_ERROR(BadValue);
		
	}
	if (!(*file = fopen(*fname, "r"))) {
		if (log) {
			mpt_log(log, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("failed to open file"), *fname);
			
		}
		return MPT_ERROR(BadArgument);
	}
	if ((ret = args->_vptr->advance(args)) < 0) {
		return ret;
	}
	return 's';
}
static int getValue(MPT_INTERFACE(metatype) *mt, FILE **file, const char **fname)
{
	int ret, type;
	
	if ((ret = MPT_metatype_convert(mt, type = MPT_ENUM(TypeFilePtr), file)) >= 0) {
		if (*file) {
			*fname = 0;
			return type;
		}
		return MPT_ERROR(MissingData);
	}
	if ((ret = MPT_metatype_convert(mt, type = 's', fname)) >= 0) {
		if (!*fname) {
			return MPT_ERROR(BadValue);
		}
	}
	if (!(*file = fopen(*fname, "r"))) {
		return MPT_ERROR(BadValue);
	}
	return type;
}

void replaceConfig(MPT_STRUCT(node) *conf, MPT_STRUCT(node) *from)
{
	mpt_node_clear(conf);
	mpt_gnode_swap(conf, from);
}

/*!
 * \ingroup mptSolver
 * \brief read solver configs
 * 
 * Read config files for client supplied by arguments.
 * If no argument(s) supplied use file names in existing config.
 * 
 * \param conf  solver client config
 * \param args  config file parameter source
 * \param info  log/error output target
 * 
 * \retval 0  no files read
 * \retval 1  new client config
 * \retval 2  new solver config
 * \retval 3  new client and solver config
 * \retval <0 error code
 */
extern int mpt_solver_read(MPT_STRUCT(node) *conf, MPT_STRUCT(iterator) *args, MPT_INTERFACE(logger) *info)
{
	static const char solconfName[] = "solconf";
	static const char fmt_client[]  = "{*} = !#";
	static const char fmt_solver[] = "[ ] = !#";
	MPT_STRUCT(node) *sol, cfg = MPT_NODE_INIT;
	const char *fname = 0;
	FILE *file = 0;
	int ret = 2, err;
	
	if (args) {
		/* create new config */
		if ((err = getArg(args, &file, &fname, info, __func__)) < 0) {
			return err;
		}
		err = mpt_node_parse(&cfg, file, fmt_client, 0, info);
		if (fname) {
			fclose(file);
		}
		if (err < 0) {
			return err;
		}
		else if (fname) {
			MPT_STRUCT(value) val = MPT_VALUE_INIT('s', &fname);
			mpt_meta_set(&conf->_meta, &val);
		}
		/* get new solver config */
		sol = mpt_node_next(cfg.children, solconfName);
		
		fname = 0;
		/* no further data */
		if (!args->_vptr->value(args)) {
			if (!sol || !sol->_meta) {
				replaceConfig(conf, &cfg);
				return 1;
			}
			args = 0;
			ret = 1; /* no 2nd argument */
		}
		/* get solver config file parameters from argument */
		else if ((ret = getArg(args, &file, &fname, info, __func__)) < 0) {
			mpt_node_clear(&cfg);
			return ret;
		}
		/* invalid trailing data */
		else if (args->_vptr->value(args)) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Critical), "%s",
				        MPT_tr("excessive config argument(s)"));
			}
			return MPT_ERROR(BadArgument);
		}
		else {
			ret = 2;
		}
	}
	/* use config value to get content source */
	else if (!conf->_meta) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Warning), "%s",
			        MPT_tr("missing client config source"));
		}
		return MPT_ERROR(BadType);
	}
	/* use config value to get content source */
	else if ((err = getValue(conf->_meta, &file, &fname)) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("bad client config source"));
		}
		return err;
	}
	/* create new config */
	else {
		err = mpt_node_parse(&cfg, file, fmt_client, 0, info);
		if (fname) {
			fclose(file);
		}
		if (err < 0) {
			return err;
		}
		/* no indirect source */
		if (!(sol = mpt_node_next(cfg.children, solconfName))
		    || !sol->_meta) {
			replaceConfig(conf, &cfg);
			return 1;
		}
	}
	
	/* get solver config file parameters from data */
	if (!args) {
		/* bad source in config */
		fname = 0;
		if ((ret = getValue(sol->_meta, &file, &fname)) < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("bad solver config source"));
			}
			mpt_node_clear(&cfg);
			return MPT_ERROR(BadType);
		}
	}
	/* require new config node */
	else if (!sol) {
		if (!(sol = mpt_node_new(strlen(solconfName) + 1))) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Critical), "%s",
				        MPT_tr("failed to create solver config node"));
			}
			mpt_node_clear(&cfg);
			return MPT_ERROR(BadOperation);
		}
		/* name and insert new node */
		mpt_identifier_set(&sol->ident, solconfName, -1);
		mpt_gnode_insert(&cfg, 0, sol);
	}
	/* add solver config to new/existing data */
	err = mpt_node_parse(sol, file, fmt_solver, 0, info);
	if (fname) {
		fclose(file);
	}
	
	/* clear created elements */
	if (err < 0) {
		mpt_node_clear(&cfg);
		return err;
	}
	else if (args && fname) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT('s', &fname);
		mpt_meta_set(&sol->_meta, &val);
	}
	/* replace client configuration witch created data */
	replaceConfig(conf, &cfg);
	
	return ret;
}
