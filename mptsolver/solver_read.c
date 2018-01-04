/*!
 * read config files
 */

#include <string.h>

#include "node.h"
#include "parse.h"

#include "meta.h"

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
	
	if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeValue), &val)) >= 0) {
		return 0;
	}
	if ((ret = mt->_vptr->conv(mt, type = 's', ptr)) >= 0) {
		return *ptr ? type : MPT_ERROR(BadValue);
	}
	if ((ret = mt->_vptr->conv(mt, type = MPT_ENUM(TypeFile), ptr)) >= 0) {
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
	int err, ret = 0;
	
	/* default setup for client config */
	val.fmt = fmt;
	val.ptr = dat;
	dat[0] = 0;
	dat[1] = fmt_cl;
	
	ret = 0;
	if (args) {
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
			return err;
		}
		/* invalid trailing data */
		else if (ret) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Critical), "%s",
				        MPT_tr("excessive config argument(s)"));
			}
			return MPT_ERROR(BadArgument);
		}
		/* indicate new config data */
		ret = 0x1;
	}
	/* use existing client config data */
	else if ((sol = conf->children)) {
		/* no solver config data required */
		if (!(sol = mpt_node_next(sol, solconfName))) {
			return 0;
		}
		/* use existing solver config data */
		if (sol->children) {
			return 0;
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
	else if ((err = getValue(conf->_meta, &val, (const void **) &dat[0])) < 0) {
		if (info) {
			mpt_log(info, __func__, MPT_LOG(Error), "%s",
			        MPT_tr("bad client config source"));
		}
		return MPT_ERROR(BadType);
	}
	/* create new config */
	else {
		fmt[0] = err;
		if ((err = mpt_node_parse(&cfg, &val, info)) < 0) {
			return err;
		}
		/* no solver config new data */
		if (!(sol = mpt_node_next(cfg.children, solconfName))) {
			replaceConfig(conf, &cfg);
			return 0x1;
		}
		/* indicate new config data */
		ret = 0x1;
	}
	
	/* get solver config file parameters from data */
	if (!args) {
		/* use existing solver config data */
		if (!sol->_meta) {
			if (info) {
				mpt_log(info, __func__, MPT_LOG(Info), "%s",
				        MPT_tr("empty solver config source"));
			}
			if (ret & 0x1) {
				replaceConfig(conf, &cfg);
			}
			return ret;
		}
		/* default setup for solver config */
		val.fmt = fmt;
		val.ptr = dat;
		dat[0] = 0;
		dat[1] = fmt_sol;
		fmt[0] = 0;
		
		if ((err = getValue(sol->_meta, &val, (const void **) &dat[0])) < 0) {
			/* bad data in new config more serious */
			ret = ret ? MPT_LOG(Error) : MPT_LOG(Warning);
			if (info) {
				mpt_log(info, __func__, ret, "%s",
				        MPT_tr("bad solver config source"));
			}
			mpt_node_clear(&cfg);
			return err;
		}
		fmt[0] = err;
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
	err = mpt_node_parse(sol, &val, info);
	ret |= 0x2;
	
	/* clear created elements */
	if (err < 0) {
		mpt_node_clear(&cfg);
		return err;
	}
	/* replace client configuration witch created data */
	if (ret & 0x1) {
		replaceConfig(conf, &cfg);
	}
	return ret;
}
