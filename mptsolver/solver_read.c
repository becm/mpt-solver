/*!
 * MPT solver library
 *   read config files
 */

#include <string.h>

#include "types.h"
#include "meta.h"
#include "node.h"
#include "parse.h"
#include "output.h"

#include "solver.h"


static int getArg(MPT_INTERFACE(iterator) *args, MPT_STRUCT(value) *val, const char **fname, MPT_INTERFACE(logger) *log, const char *_func)
{
	int ret;
	if ((ret = args->_vptr->get(args, MPT_ENUM(TypeValue), val)) < 0) {
		if ((ret = args->_vptr->get(args, 's', fname)) < 0
		    || !*fname) {
			ret = args->_vptr->get(args, 0, 0);
			mpt_log(log, _func, MPT_LOG(Error), "%s (%d)",
			        MPT_tr("invalid argument type"), ret);
			return MPT_ERROR(BadValue);
		}
	}
	return args->_vptr->advance(args);
}
static int getValue(MPT_INTERFACE(metatype) *mt, MPT_STRUCT(value) *val, const void **ptr)
{
	int ret, type;
	
	if ((ret = MPT_metatype_convert(mt, MPT_ENUM(TypeValue), &val)) >= 0) {
		return 0;
	}
	if ((ret = MPT_metatype_convert(mt, type = MPT_ENUM(TypeFilePtr), ptr)) >= 0) {
		return *ptr ? type : MPT_ERROR(BadValue);
	}
	if ((ret = MPT_metatype_convert(mt, type = 's', ptr)) >= 0) {
		return *ptr ? type : MPT_ERROR(BadValue);
	}
	return MPT_ERROR(BadType);
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
	static const char fmt_cl[]  = "{*} = !#";
	static const char fmt_sol[] = "[ ] = !#";
	MPT_STRUCT(node) *sol, cfg = MPT_NODE_INIT;
	MPT_STRUCT(value) val;
	uint8_t fmt[] = "ss";
	const char *dat[2];
	int ret;
	
	/* default setup for client config */
	val.fmt = fmt;
	val.ptr = dat;
	dat[0] = 0;
	dat[1] = fmt_cl;
	
	if (args) {
		int err;
		
		/* get client config file parameters from argument */
		if ((ret = getArg(args, &val, &dat[0], info, __func__)) < 0) {
			return ret;
		}
		/* create new config */
		if ((err = mpt_node_parse(&cfg, &val, info)) < 0) {
			return err;
		}
		/* get new solver config */
		dat[0] = 0;
		dat[1] = fmt_sol;
		sol = mpt_node_next(cfg.children, solconfName);
		
		/* no further data */
		if (!ret) {
			if (!sol || !sol->_meta) {
				replaceConfig(conf, &cfg);
				return 1;
			}
			args = 0;
		}
		/* get solver config file parameters from argument */
		else if ((ret = getArg(args, &val, &dat[0], info, __func__)) < 0) {
			mpt_node_clear(&cfg);
			return ret;
		}
		/* invalid trailing data */
		else if (ret) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Critical), "%s",
				        MPT_tr("excessive config argument(s)"));
			}
			return MPT_ERROR(BadArgument);
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
	else if ((ret = getValue(conf->_meta, &val, (const void **) &dat[0])) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("bad client config source"));
		}
		return MPT_ERROR(BadType);
	}
	/* create new config */
	else {
		if (ret) {
			fmt[0] = ret;
		}
		if ((ret = mpt_node_parse(&cfg, &val, info)) < 0) {
			return ret;
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
		/* default setup for solver config */
		val.fmt = fmt;
		val.ptr = dat;
		dat[0] = 0;
		dat[1] = fmt_sol;
		
		/* bad source in config */
		if ((ret = getValue(sol->_meta, &val, (const void **) &dat[0])) < 0) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Error), "%s",
				        MPT_tr("bad solver config source"));
			}
			mpt_node_clear(&cfg);
			return MPT_ERROR(BadType);
		}
		if (ret) {
			fmt[0] = ret;
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
	ret = mpt_node_parse(sol, &val, info);
	
	/* clear created elements */
	if (ret < 0) {
		mpt_node_clear(&cfg);
		return ret;
	}
	/* replace client configuration witch created data */
	replaceConfig(conf, &cfg);
	
	return 2;
}
