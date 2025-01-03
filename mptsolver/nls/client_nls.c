/*!
 * MPT solver library
 *   create client for solving nonlinear systems
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <sys/resource.h>

#include "convert.h"
#include "node.h"

#include "parse.h"
#include "client.h"
#include "config.h"
#include "event.h"

#include "values.h"

#include "solver.h"

MPT_STRUCT(NLS) {
	MPT_INTERFACE(client) _cl;
	MPT_INTERFACE(config) _cfg;
	
	MPT_STRUCT(solver_data) *sd;
	MPT_INTERFACE(metatype) *cfg;
	MPT_INTERFACE(metatype) *sol;
	
	MPT_SOLVER_TYPE(UserInit) *uinit;
	
	struct {
		struct timeval usr,  /* user time in solver backend */
		               sys;  /* system time in solver backend */
	} ru;
};
struct _outNLSdata
{
	MPT_STRUCT(solver_output) *out;
	MPT_STRUCT(solver_data) *dat;
	int state;
};

static int outNLS(void *ptr, const MPT_STRUCT(value) *val)
{
	const struct _outNLSdata *ctx = ptr;
	int ret;
	
	/* copy parameters form solver output */
	if ((ret = mpt_solver_data_nls(ctx->dat, val)) < 0) {
		return ret;
	}
	/* output of user data and residuals */
	if (ctx->out) {
		return mpt_solver_output_nls(ctx->out, ctx->state, val);
	}
	return ret;
}
static MPT_INTERFACE(logger) *loggerNLS(const MPT_STRUCT(NLS) *nls)
{
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *info;
	
	if (nls
	    && (mt = nls->cfg)) {
		MPT_INTERFACE(config) *cfg = 0;
		if (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg) > 0
		    && cfg
		    && (info = mpt_config_logger(cfg))) {
			return info;
		}
	}
	/* use global config log target */
	if (!(info = mpt_config_logger(0))) {
		/* fallback to default logger */
		info = mpt_log_default();
	}
	return info;
}

static MPT_INTERFACE(config) *configNLS(MPT_STRUCT(NLS) *nls)
{
	MPT_INTERFACE(metatype) *cfg;
	MPT_INTERFACE(config) *sub = 0;
	
	if (!(cfg = nls->cfg)) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		mpt_path_set(&p, "mpt.config", -1);
		if (!(cfg = mpt_config_global(&p))) {
			return 0;
		}
		nls->cfg = cfg;
	}
	/* mitigation: trigger node creation */
	if (MPT_metatype_convert(cfg, MPT_ENUM(TypeNodePtr), 0) < 0) {
		return 0;
	}
	
	if (MPT_metatype_convert(cfg, MPT_ENUM(TypeConfigPtr), &sub) < 0) {
		return 0;
	}
	return sub;
}
/* config interface */
static int queryNLS(const MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *p, MPT_TYPE(config_handler) fcn, void *ctx)
{
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_INTERFACE(config) *conf;
	
	if (!p) {
		return fcn ? fcn(ctx, (MPT_INTERFACE(convertable) *) nls->sol, 0) : 0;
	}
	if (!(conf = configNLS(nls))) {
		return MPT_ERROR(BadValue);
	}
	return conf->_vptr->query(conf, p, fcn, ctx);
}
static int assignNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val)
{
	static const char _func[] = "mpt::client<NLS>::assign";
	
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_INTERFACE(config) *conf;
	const char *path;
	int ret;
	
	if (!porg) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		if (nls->cfg) {
			return MPT_ERROR(BadOperation);
		}
		if (!val) {
			return configNLS(nls) ? 0 : MPT_ERROR(BadOperation);
		}
		path = 0;
		if (mpt_value_convert(val, 's', &path) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!mpt_path_set(&p, path, -1)) {
			return MPT_ERROR(BadValue);
		}
		if (!(nls->cfg = mpt_config_global(&p))) {
			return MPT_ERROR(MissingData);
		}
		return 0;
	}
	if (!(conf = configNLS(nls))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg->len) {
		MPT_INTERFACE(logger) *info;
		MPT_STRUCT(node) *root;
		FILE *file;
		
		/* external log target only */
		info = loggerNLS(0);
		
		root = 0;
		if (MPT_metatype_convert(nls->cfg, MPT_ENUM(TypeNodePtr), &root) < 0
		 || !root) {
			mpt_log(info, _func, MPT_LOG(Critical), "%s: %s",
			        MPT_tr("missing node structure"),
			        val->_type);
			return MPT_ERROR(BadType);
		}
		
		path = 0;
		if (mpt_value_convert(val, 's', &path) < 0 || path == 0) {
			int type = val->_type;
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("invalid file name type"),
			        type);
			return MPT_ERROR(BadType);
		}
		if (!(file = fopen(path, "r"))) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("failed to open client config"),
			        path);
			return MPT_ERROR(BadValue);
		}
		ret = mpt_node_parse(root, file, 0, 0, info);
		fclose(file);
		if (ret < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("failed to load client config"));
		} else {
			MPT_STRUCT(value) val = MPT_VALUE_INIT('s', &path);
			mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s: %s",
			        MPT_tr("loaded client config file"), path);
			mpt_meta_set(&root->_meta, &val);
		}
		return ret;
	}
	if ((ret = conf->_vptr->assign(conf, porg, val)) < 0) {
		mpt_log(loggerNLS(nls), _func, MPT_LOG(Critical), "%s",
		        MPT_tr("unable to assign client element"));
	}
	return ret;
}
static int removeNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *p)
{
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_STRUCT(config) *conf;
	
	if (!p) {
		MPT_INTERFACE(metatype) *mt;
		if (nls->sd) {
			mpt_solver_data_clear(nls->sd);
		}
		if (!(mt = nls->sol)) {
			return 0;
		}
		mt->_vptr->unref(mt);
		nls->sol = 0;
		return 1;
	}
	if (!(conf = configNLS(nls))) {
		return MPT_ERROR(BadOperation);
	}
	return conf->_vptr->remove(conf, p);
}
/* convertable interface */
static int convNLS(MPT_INTERFACE(convertable) *val, MPT_TYPE(type) type, void *ptr)
{
	MPT_STRUCT(NLS) *nls = (void *) val;
	const MPT_STRUCT(named_traits) *traits = mpt_client_type_traits();
	MPT_TYPE(type) me = traits ? traits->type : MPT_ENUM(TypeMetaPtr);
	
	if (traits && traits->type == type) {
		if (ptr) *((const void **) ptr) = &nls->_cl;
		return MPT_ENUM(TypeConfigPtr);
	}
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeConfigPtr), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = fmt;
		}
		return me;
	}
	if (type == MPT_ENUM(TypeConfigPtr)) {
		if (ptr) *((const void **) ptr) = &nls->_cfg;
		return me;
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void deleteNLS(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(NLS) *nls = (void *) mt;
	
	if (nls->sd) {
		mpt_solver_data_fini(nls->sd);
		free(nls->sd);
	}
	if ((mt = nls->sol)) {
		mt->_vptr->unref(mt);
	}
	if ((mt = nls->cfg)) {
		mt->_vptr->unref(mt);
	}
	free(nls);
}
static uintptr_t addrefNLS(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *cloneNLS(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* init operation for solver */
static int initNLS(MPT_STRUCT(NLS) *nls, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<NLS>::init";
	
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(solver_data) *dat;
	MPT_STRUCT(node) *conf, *curr;
	MPT_INTERFACE(object) *obj;
	MPT_INTERFACE(logger) *info, *hist;
	MPT_SOLVER(interface) *sol;
	const char *val;
	int ret;
	
	(void) args;
	
	info = hist = loggerNLS(0);
	
	if (!configNLS(nls)) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("unable to get NLS client config"));
		return MPT_ERROR(BadOperation);
	}
	if (MPT_metatype_convert(nls->cfg, MPT_ENUM(TypeNodePtr), &conf) < 0
	 || !conf) {
		mpt_log(info, _func, MPT_LOG(Critical), "%s: %s",
		        MPT_tr("failed query"), MPT_tr("IVP config node"));
		return MPT_ERROR(BadOperation);
	}
	
	if ((curr = mpt_node_find(conf, "output", 1))) {
		MPT_INTERFACE(metatype) *old;
		mt = mpt_output_local();
		obj = 0;
		MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj);
		mpt_conf_history(obj, curr);
		if ((old = curr->_meta)) {
			old->_vptr->unref(old);
		}
		curr->_meta = mt;
		hist = loggerNLS(nls);
	}
	/* clear/create solver data */
	if ((dat = nls->sd)) {
		mpt_solver_data_clear(dat);
	}
	else if ((dat = malloc(sizeof(*dat)))) {
		const MPT_STRUCT(solver_data) sd = MPT_SOLVER_DATA_INIT;
		*dat = sd;
		nls->sd = dat;
	}
	else {
		mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to allocate memory"), MPT_tr("solver data"));
		return MPT_ERROR(BadOperation);
	}
	/* assign nls data */
	if ((ret = mpt_conf_nls(dat, conf->children, info)) < 0) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("solver preparation failed"));
		return ret;
	}
	/* set graphic parameters */
	if ((curr = mpt_node_find(conf, "graphic", 1))) {
		MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
		mpt_solver_output_query(&out, &nls->_cfg);
		mpt_solver_output_query(&out, 0);
		if (out._graphic) {
			mpt_conf_graphic(out._graphic, conf->children, info);
			mpt_solver_output_nlsdata(&out, MPT_DATASTATE(Init), dat);
		}
		mpt_array_clone(&out._pass, 0);
	}
	/* load new solver or evaluate existing */
	curr = mpt_node_find(conf, "solver", 1);
	val = curr ? mpt_node_data(curr, 0) : 0;
	if (!(sol = mpt_solver_load(&nls->sol, MPT_SOLVER_ENUM(CapableNls), val, info))) {
		return MPT_ERROR(BadValue);
	}
	if (args) {
		double *par = (void *) (dat->param._buf + 1);
		int i;
		for (i = 0; i < dat->npar; ++i) {
			const MPT_STRUCT(value) *val = args->_vptr->value(args);
			if (!val || (ret = mpt_value_convert(val, 'd', par + i)) < 0) {
				mpt_log(info, _func, MPT_LOG(Warning), "%s: %d",
				        MPT_tr("bad initial value"), i + 1);
			}
			if ((ret = args->_vptr->advance(args)) < 0) {
				mpt_log(info, _func, MPT_LOG(Warning), "%s: %d",
				        MPT_tr("unable to advance initial values"), i);
				break;
			}
			if (!ret) {
				break;
			}
		}
	}
	obj = 0;
	if ((mt = nls->sol)
	    && MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj) > 0
	    && obj
	    && (val = mpt_object_typename(obj))) {
		mpt_log(hist, 0, MPT_LOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	if ((ret = nls->uinit((MPT_INTERFACE(convertable) *) mt, nls->sd, hist)) < 0) {
		return ret;
	}
	memset(&nls->ru, 0, sizeof(nls->ru));
	mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
	        MPT_tr("NLS client setup finished"));
	
	return 0;
}
/* prepare operation on solver */
static int prepNLS(MPT_STRUCT(NLS) *nls, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<NLS>::prep";
	
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *info, *hist;
	MPT_INTERFACE(object) *obj;
	MPT_SOLVER(interface) *sol;
	MPT_STRUCT(node) *cfg;
	int err;
	
	info = loggerNLS(0);
	
	if (!(mt = nls->sol)) {
		mpt_log(info, _func, MPT_LOG(Warning), "%s (%" PRIxPTR ")",
		        MPT_tr("no solver configured"), nls);
		return MPT_ERROR(BadOperation);
	}
	sol = 0;
	MPT_metatype_convert(mt, MPT_ENUM(TypeSolverPtr), &sol);
	
	obj = 0;
	if (MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj) < 0
	    || !obj) {
		err = MPT_metatype_convert(mt, 0, 0);
		mpt_log(info, _func, MPT_LOG(Warning), "%s (%s = %d)",
		        MPT_tr("solver without object interface"), MPT_tr("type"), err);
		return MPT_ERROR(BadOperation);
	}
	hist = loggerNLS(nls);
	
	/* set solver parameters from config */
	if (configNLS(nls)
	 && MPT_metatype_convert(nls->cfg, MPT_ENUM(TypeNodePtr), &cfg)
	 && cfg
	 && (cfg = cfg->children)) {
		mpt_solver_param(obj, cfg, hist);
	}
	/* set solver parameters from args */
	if (args && (err = mpt_object_args(obj, args)) < 0) {
		mpt_log(info, _func, MPT_LOG(Warning), "%s",
		        MPT_tr("failed to apply solver parameters"));
		return err;
	}
	err = obj->_vptr->set_property(obj, 0, 0);
	if (sol) {
		mpt_solver_info(sol, hist);
	}
	if (err < 0) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("solver prepare failed"));
	} else {
		mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
		        MPT_tr("IVP client preparation finished"));
	}
	return err;
}
/* step operation for solver */
static int stepNLS(MPT_STRUCT(NLS) *nls, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<NLS>::step";
	
	const MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(solver_data) *dat;
	MPT_SOLVER(interface) *sol;
	MPT_STRUCT(node) *names;
	struct _outNLSdata ctx;
	struct rusage pre, post;
	const double *par;
	MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
	MPT_INTERFACE(logger) *info, *hist;
	int res;
	
	(void) args;
	
	info = loggerNLS(0);
	
	if (!(mt = nls->sol) || !(dat = nls->sd)) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("solver or data missing"));
		return MPT_ERROR(BadArgument);
	}
	if ((res = MPT_metatype_convert(mt, MPT_ENUM(TypeSolverPtr), &sol)) < 0
	    || !sol) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("no solver interface"));
		return MPT_ERROR(BadArgument);
	}
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	
	/* trigger solver iteration */
	res = sol->_vptr->solve(sol);
	
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_timeradd_sys(&nls->ru.sys, &pre, &post);
	mpt_timeradd_usr(&nls->ru.usr, &pre, &post);
	
	hist = loggerNLS(nls);
	if (!hist) {
		return res;
	}
	ctx.out = &out;
	ctx.dat = dat;
	
	/* select output state */
	if (res < 0) {
		ctx.state = MPT_DATASTATE(Fail);
	} else if (res) {
		ctx.state = MPT_DATASTATE(Step);
	} else {
		ctx.state = MPT_DATASTATE(Fini) | MPT_DATASTATE(Step);
	}
	mpt_solver_output_query(&out, &nls->_cfg);
	mpt_solver_output_query(&out, 0);
	mpt_solver_status(sol, hist, outNLS, &ctx);
	mpt_array_clone(&out._pass, 0);
	
	/* no final output for state */
	if (res > 0) {
		return res;
	}
	
	names = 0;
	if (configNLS(nls)
	 && (MPT_metatype_convert(nls->cfg, MPT_ENUM(TypeNodePtr), &names) >= 0)
	 && names
	 && (names = mpt_node_find(names, "param", 1))) {
		names = names->children;
	}
	if ((par = mpt_solver_data_param(dat))) {
		int i, npar;
		
		npar = nls->sd->npar;
		for (i = 0; i < npar; ++i) {
			const char *desc = MPT_tr("parameter");
			
			if (names) {
				const char *name = mpt_node_ident(names);
				names = names->next;
				if (name) {
					mpt_log(hist, 0, MPT_LOG(Message), "%s %2d: %16g (%s)",
					        desc, i + 1, par[i], name);
					continue;
				}
			}
			mpt_log(hist, 0, MPT_LOG(Message), "%s %2d: %16g",
			        desc, i + 1, par[i]);
		}
	}
	mpt_solver_statistics(sol, hist, &nls->ru.usr, &nls->ru.sys);
	
	return res;
}
/* client interface */
static int processNLS(MPT_INTERFACE(client) *cl, uintptr_t id, MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(NLS) *nls = (void *) cl;
	int ret;
	
	/* ignore empty iterator */
	if (it && !it->_vptr->value(it)) {
		it = 0;
	}
	
	if (!id || id == mpt_hash("start", 5)) {
		MPT_STRUCT(node) *cfg = 0;
		if (!configNLS(nls)
		 || MPT_metatype_convert(nls->cfg, MPT_ENUM(TypeNodePtr), &cfg) < 0
		 || !cfg) {
			return MPT_ERROR(BadOperation);
		}
		if (id && (ret = mpt_solver_read(cfg, it, loggerNLS(0))) < 0) {
			return ret;
		}
		if ((ret = initNLS(nls, id ? 0 : it)) < 0) {
			return ret;
		}
		if ((ret = prepNLS(nls, 0)) < 0) {
			return ret;
		}
		return MPT_EVENTFLAG(Default);
	}
	else if (id == mpt_hash("read", 4)) {
		MPT_STRUCT(node) *cfg = 0;
		if (!configNLS(nls)
		 || MPT_metatype_convert(nls->cfg, MPT_ENUM(TypeNodePtr), &cfg) < 0
		 || !cfg) {
			return MPT_ERROR(BadOperation);
		}
		ret = mpt_solver_read(cfg, it, loggerNLS(0));
	}
	else if (id == mpt_hash("init", 4)) {
		ret = initNLS(nls, it);
	}
	else if (id == mpt_hash("prep", 4)) {
		ret = prepNLS(nls, it);
	}
	else if (id == mpt_hash("step", 4)) {
		ret = stepNLS(nls, it);
	}
	else if (id == mpt_hash("cont", 4)) {
		return MPT_EVENTFLAG(Default);
	}
	else if (id == mpt_hash("set", 3)) {
		ret = mpt_config_args(&nls->_cfg, it, loggerNLS(0));
	}
	else if (id == mpt_hash("unset", 5) || id == mpt_hash("del", 3)) {
		ret = mpt_config_clear(&nls->_cfg, it, loggerNLS(0));
	}
	else {
		return MPT_ERROR(BadArgument);
	}
	if (ret < 0) {
		return MPT_EVENTFLAG(Fail) | MPT_EVENTFLAG(Default);
	} else {
		return MPT_EVENTFLAG(None);
	}
}
static int dispatchNLS(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(NLS) *nls = (void *) cl;
	int err;
	
	if (!ev) {
		if ((err = stepNLS(nls, 0)) < 0) {
			return err;
		}
		return err ? MPT_EVENTFLAG(Default) : MPT_EVENTFLAG(Terminate);
	}
	if (!ev->msg) {
		if ((err = stepNLS(nls, 0)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad step operation"));
		}
		if (err & MPT_EVENTFLAG(Fail)) {
			mpt_context_reply(ev->reply, err, "%s (%" PRIxPTR ")",
			                  MPT_tr("step operation error"), ev->id);
			ev->id = 0;
			return err | MPT_EVENTFLAG(Default);
		}
	}
	return mpt_solver_dispatch(cl, ev);
}
/*!
 * \ingroup mptSolver
 * \brief NLS client creation
 * 
 * Create client for solving Nonlinear Systems.
 * 
 * \param uinit user initialization function
 * \param base  global config path (dot delimited)
 * 
 * \return NLS client
 */
extern MPT_INTERFACE(client) *mpt_client_nls(MPT_SOLVER_TYPE(UserInit) uinit)
{
	static MPT_INTERFACE_VPTR(config) configNLS = {
		queryNLS, assignNLS, removeNLS
	};
	static MPT_INTERFACE_VPTR(client) clientNLS = {
		{ { convNLS }, deleteNLS, addrefNLS, cloneNLS },
		dispatchNLS,
		processNLS
	};
	MPT_STRUCT(NLS) *nls;
	
	if (!uinit) {
		return 0;
	}
	if (!(nls = malloc(sizeof(*nls)))) {
		return 0;
	}
	nls->_cl._vptr = &clientNLS;
	nls->_cfg._vptr = &configNLS;
	
	nls->sd = 0;
	nls->cfg = 0;
	nls->sol = 0;
	nls->uinit = uinit;
	
	memset(&nls->ru, 0, sizeof(nls->ru));
	
	return &nls->_cl;
}

