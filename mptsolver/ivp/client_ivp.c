/*!
 * MPT solver library
 *   create client for IVP problem types
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include <sys/resource.h>

#include "array.h"
#include "message.h"
#include "meta.h"
#include "node.h"

#include "convert.h"
#include "client.h"
#include "config.h"
#include "event.h"
#include "parse.h"

#include "values.h"

#include "solver.h"

MPT_STRUCT(IVP) {
	MPT_INTERFACE(client) _cl;
	MPT_INTERFACE(config) _cfg;
	
	MPT_STRUCT(solver_data) *sd;
	MPT_INTERFACE(metatype) *cfg;
	MPT_INTERFACE(metatype) *sol;
	
	MPT_SOLVER_TYPE(UserInit) *uinit;
	
	struct {
		struct timeval usr,   /* user time in solver backend */
		               sys;   /* system time in solver backend */
	} ru;
};

static MPT_INTERFACE(logger) *loggerIVP(const MPT_STRUCT(IVP) *ivp)
{
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *info;
	
	/* get local config log target */
	if (ivp
	 && (mt = ivp->cfg)) {
		MPT_INTERFACE(config) *cfg = 0;
		if (MPT_metatype_convert(mt, MPT_ENUM(TypeConfigPtr), &cfg) > 0
		 && cfg
		 && (info = mpt_config_logger(cfg))) {
			return info;
		}
	}
	/* use global config log target */
	if ((info = mpt_config_logger(0))) {
		return info;
	}
	/* fallback to default logger */
	return mpt_log_default();
}

static int setTime(void *ptr, const MPT_STRUCT(property) *pr)
{
	int ret;
	
	if (!pr || pr->name) {
		return 0;
	}
	if (!pr->val._addr) {
		return MPT_ERROR(MissingData);
	}
	if ((ret = mpt_value_convert(&pr->val, 'd', ptr)) >= 0) {
		return 0;
	}
	if (pr->val._type == MPT_ENUM(TypeObjectPtr)) {
		MPT_INTERFACE(object) *obj = *((void * const *) pr->val._addr);
		MPT_STRUCT(property) tmp = MPT_PROPERTY_INIT;
		tmp.name = "t";
		if (!obj || (ret = obj->_vptr->property(obj, &tmp)) < 0) {
			return MPT_ERROR(BadValue);
		}
		if ((ret = mpt_value_convert(&tmp.val, 'd', ptr)) < 0) {
			return ret;
		}
	}
	return 1;
}
static int getTime(MPT_SOLVER(interface) *sol, double *t)
{
	return sol->_vptr->report(sol, MPT_SOLVER_ENUM(Values), setTime, t);
}

static int nextTime(MPT_INTERFACE(iterator) *src, double *end, const char *fcn, void *arg, MPT_INTERFACE(logger) *info)
{
	double ref = *end;
	
	while (1) {
		const MPT_STRUCT(value) *curr;
		double next;
		int ret;
		
		if (!(curr = src->_vptr->value(src))) {
			mpt_log(info, fcn, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("bad value on time source"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"));
			return MPT_ERROR(MissingData);
		}
		if ((ret = mpt_value_convert(curr, 'd', &next)) < 0) {
			int type = curr->_type;
			mpt_log(info, fcn, MPT_LOG(Error), "%s (%s): %d",
			        MPT_tr("bad type on time source"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"),
			        type);
			return ret;
		}
		if (ref < next) {
			*end = next;
			return curr->_type;
		}
		mpt_log(info, fcn, MPT_LOG(Info), "%s (%g <= %g)",
		        MPT_tr("skip time value argument"), next, ref);
		
		if ((ret = src->_vptr->advance(src)) < 0) {
			mpt_log(info, fcn, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("bad time source state"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"));
			return ret;
		}
		if (!ret) {
			mpt_log(info, fcn, MPT_LOG(Warning), "%s (%s)",
			        MPT_tr("no further data in time source"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"));
			return 0;
		}
	}
}

/* output for PDE solvers */
struct _clientPdeOut
{
	MPT_STRUCT(solver_output) *out;
	MPT_STRUCT(solver_data) *dat;
	int state;
};
static int outPDE(void *ptr, const MPT_STRUCT(value) *val)
{
	struct _clientPdeOut *ctx = ptr;
	return mpt_solver_output_pde(ctx->out, ctx->state, val, ctx->dat);
}
/* config interface */
static MPT_INTERFACE(config) *configIVP(MPT_STRUCT(IVP) *ivp)
{
	MPT_INTERFACE(metatype) *cfg;
	MPT_INTERFACE(config) *sub;
	
	if (!(cfg = ivp->cfg)) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		mpt_path_set(&p, "mpt.config", -1);
		if (!(cfg = mpt_config_global(&p))) {
			return 0;
		}
		ivp->cfg = cfg;
	}
	/* mitigation: trigger node creation */
	if (MPT_metatype_convert(cfg, MPT_ENUM(TypeNodePtr), 0) < 0) {
		return 0;
	}
	
	sub = 0;
	if (MPT_metatype_convert(cfg, MPT_ENUM(TypeConfigPtr), &sub) < 0) {
		return 0;
	}
	return sub;
}
static int queryIVP(const MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *p, MPT_TYPE(config_handler) fcn, void *ctx)
{
	MPT_STRUCT(IVP) *ivp = MPT_baseaddr(IVP, gen, _cfg);
	MPT_INTERFACE(config) *conf;
	
	if (!p) {
		return fcn ? fcn(ctx, (MPT_INTERFACE(convertable) *) ivp->sol, 0) : 0;
	}
	if (!(conf = configIVP(ivp))) {
		return MPT_ERROR(BadValue);
	}
	return conf->_vptr->query(conf, p, fcn, ctx);
}
static int assignIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val)
{
	static const char _func[] = "mpt::client<IVP>::assign";
	MPT_STRUCT(IVP) *ivp = MPT_baseaddr(IVP, gen, _cfg);
	MPT_INTERFACE(config) *conf;
	const char *path;
	int ret;
	
	if (!porg) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		if (ivp->cfg) {
			return MPT_ERROR(BadOperation);
		}
		if (!val) {
			return configIVP(ivp) ? 0 : MPT_ERROR(BadOperation);
		}
		path = 0;
		if (mpt_value_convert(val, 's', &path) < 0) {
			return MPT_ERROR(BadType);
		}
		if (mpt_path_set(&p, path, -1) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (!(ivp->cfg = mpt_config_global(&p))) {
			return MPT_ERROR(MissingData);
		}
		return 0;
	}
	if (!(conf = configIVP(ivp))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg->len) {
		MPT_INTERFACE(logger) *info;
		MPT_STRUCT(node) *root;
		FILE *file;
		
		/* external log target only */
		info = loggerIVP(0);
		
		root = 0;
		if (MPT_metatype_convert(ivp->cfg, MPT_ENUM(TypeNodePtr), &root) < 0
		 || !root) {
			mpt_log(info, _func, MPT_LOG(Critical), "%s: %s",
			        MPT_tr("missing node structure"),
			        val->_type);
			return MPT_ERROR(BadType);
		}
		
		path = 0;
		if (mpt_value_convert(val, 's', &path) < 0 || path == 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("invalid filename type"),
			        val->_type);
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
		}
		else {
			MPT_STRUCT(value) val = MPT_VALUE_INIT('s', &path);
			mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s: %s",
			        MPT_tr("loaded client config file"), path);
			mpt_meta_set(&root->_meta, &val);
		}
		return ret;
	}
	if ((ret = conf->_vptr->assign(conf, porg, val)) < 0) {
		mpt_log(loggerIVP(ivp), _func, MPT_LOG(Critical), "%s",
		        MPT_tr("unable to assign client element"));
	}
	return ret;
}
static int removeIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *p)
{
	MPT_STRUCT(IVP) *ivp = MPT_baseaddr(IVP, gen, _cfg);
	MPT_STRUCT(config) *conf;
	
	if (!p) {
		MPT_INTERFACE(metatype) *mt;
		if (ivp->sd) {
			mpt_solver_data_clear(ivp->sd);
		}
		if (!(mt = ivp->sol)) {
			return 0;
		}
		mt->_vptr->unref(mt);
		ivp->sol = 0;
		return 1;
	}
	if (!(conf = configIVP(ivp))) {
		return MPT_ERROR(BadOperation);
	}
	return conf->_vptr->remove(conf, p);
}
/* convertable interface */
static int convIVP(MPT_INTERFACE(convertable) *val, MPT_TYPE(type) type, void *ptr)
{
	MPT_STRUCT(IVP) *ivp = (void *) val;
	const MPT_STRUCT(named_traits) *traits = mpt_client_type_traits();
	MPT_TYPE(type) me = traits ? traits->type : MPT_ENUM(TypeMetaPtr);
	
	if (traits && traits->type == type) {
		if (ptr) *((const void **) ptr) = &ivp->_cl;
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
		if (ptr) *((const void **) ptr) = &ivp->_cfg;
		return me;
	}
	return MPT_ERROR(BadType);
}
/* metatype interface */
static void deleteIVP(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(IVP) *ivp = (void *) mt;
	
	if (ivp->sd) {
		mpt_solver_data_fini(ivp->sd);
		free(ivp->sd);
	}
	if ((mt = ivp->sol)) {
		mt->_vptr->unref(mt);
	}
	if ((mt = ivp->cfg)) {
		mt->_vptr->unref(mt);
	}
	free(ivp);
}
static uintptr_t addrefIVP(MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
static MPT_INTERFACE(metatype) *cloneIVP(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* init operation for solver */
static int initIVP(MPT_STRUCT(IVP) *ivp, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<IVP>::init";
	
	MPT_STRUCT(node) *conf, *curr;
	MPT_STRUCT(solver_data) *dat;
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(object) *obj;
	MPT_INTERFACE(logger) *info, *hist;
	MPT_SOLVER(interface) *sol;
	const char *val;
	double t;
	int ret;
	
	info = hist = loggerIVP(0);
	
	if (!configIVP(ivp)) {
		mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to query"), MPT_tr("client configuration"));
		return MPT_ERROR(BadOperation);
	}
	if (MPT_metatype_convert(ivp->cfg, MPT_ENUM(TypeNodePtr), &conf) < 0
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
		hist = loggerIVP(ivp);
	}
	/* clear/create solver data */
	if ((dat = ivp->sd)) {
		mpt_solver_data_clear(dat);
	}
	else if ((dat = malloc(sizeof(*dat)))) {
		ivp->sd = memset(dat, 0, sizeof(*dat));
	}
	else {
		mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to allocate memory"), MPT_tr("solver data"));
		return MPT_ERROR(BadOperation);
	}
	/* set data and adapt config elements */
	if ((ret = mpt_conf_ivp(dat, conf->children, args ? args->_vptr->value(args) : 0, info)) < 0) {
		return ret;
	}
	/* get time source from config */
	if (!args
	 && (curr = mpt_node_next(conf->children, "times"))
	 && (mt = curr->_meta)) {
		ret = MPT_metatype_convert(mt, MPT_ENUM(TypeIteratorPtr), &args);
	}
	/* query and advance time source */
	t = 0.0;
	if (args) {
		if ((ret = mpt_iterator_consume(args, 'd', &t)) < 0) {
			mpt_log(info, __func__, MPT_LOG(Warning), "%s: %s",
			        MPT_tr("unable to advance iterator"), MPT_tr("time steps"));
		}
	}
	/* set graphic parameters */
	if ((curr = mpt_node_find(conf, "graphic", 1))) {
		MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
		
		mpt_solver_output_query(&out, &ivp->_cfg);
		mpt_solver_output_query(&out, 0);
		if (out._graphic) {
			mpt_conf_graphic(out._graphic, conf->children, info);
		}
		mpt_array_clone(&out._pass, 0);
	}
	/* load new solver or evaluate existing */
	curr = mpt_node_find(conf, "solver", 1);
	val = curr ? mpt_node_data(curr, 0) : 0;
	if (!(sol = mpt_solver_load(&ivp->sol, MPT_SOLVER_ENUM(CapableIvp), val, info))) {
		return MPT_ERROR(BadValue);
	}
	obj = 0;
	if (!(mt = ivp->sol)
	 || MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj) < 0
	 || !obj) {
		mpt_log(info, _func, MPT_LOG(Error), "%s (%" PRIxPTR ")",
		        MPT_tr("solver without object interface"), sol);
		return MPT_ERROR(BadValue);
	}
	val = mpt_object_typename(obj);
	if (val) {
		mpt_log(hist, 0, MPT_LOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	
	if ((ret = ivp->uinit((MPT_INTERFACE(convertable) *) mt, dat, hist)) < 0) {
		return ret;
	}
	/* setup PDE time and profile data */
	if (!dat->nval) {
		/* no profile data requested or available */
		if (!ret
		    || !(curr = mpt_node_next(conf->children, "profile"))
		    || !(mt = curr->_meta)) {
			if ((ret = mpt_solver_setvalue(obj, 0, t)) < 0) {
				mpt_log(info, 0, MPT_LOG(Error), "%s: %s",
				        MPT_tr("solver"), MPT_tr("failed to set initial time"));
				return ret;
			}
		}
		/* process profile data */
		else if ((ret = obj->_vptr->set_property(obj, 0, (MPT_INTERFACE(convertable) *) mt)) < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("PDE init state assignment failed"));
			return ret;
		}
	}
	else if (ret++) {
		double *post;
		if (ret <= dat->nval) {
			dat->nval = ret;
			dat->val._buf->_used = ret * sizeof(double);
		}
		else if (!(post = mpt_values_prepare(&dat->val, dat->nval - ret))) {
			return MPT_ERROR(BadOperation);
		}
		if ((ret = mpt_ivp_data(obj, &dat->val, info)) < 0) {
			return ret;
		}
	}
	memset(&ivp->ru, 0, sizeof(ivp->ru));
	mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
	        MPT_tr("IVP client setup finished"));
	
	return 0;
}
/* prepare operation on solver */
static int prepIVP(MPT_STRUCT(IVP) *ivp, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<IVP>::prep";
	
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *info, *hist;
	MPT_INTERFACE(object) *obj;
	MPT_SOLVER(interface) *sol;
	MPT_STRUCT(node) *cfg = 0;
	int ret, err;
	
	info = loggerIVP(0);
	
	if (!(mt = ivp->sol)) {
		mpt_log(info, _func, MPT_LOG(Warning), "%s",
		        MPT_tr("no solver configured"));
		return MPT_ERROR(BadOperation);
	}
	sol = 0;
	if ((err = MPT_metatype_convert(mt, MPT_ENUM(TypeSolverPtr), &sol)) < 0
	 || !sol) {
		err = MPT_metatype_convert(mt, 0, 0);
		mpt_log(info, _func, MPT_LOG(Warning), "%s (%s = %d)",
		        MPT_tr("no solver interface"), MPT_tr("type"), err);
		return MPT_ERROR(BadType);
	}
	obj = 0;
	if ((err = MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj)) < 0
	 || !obj) {
		err = MPT_metatype_convert(mt, 0, 0);
		mpt_log(info, _func, MPT_LOG(Warning), "%s (%s = %d)",
		        MPT_tr("solver without object interface"), MPT_tr("type"), err);
		return MPT_ERROR(BadType);
	}
	hist = loggerIVP(ivp);
	
	/* set solver parameters from config */
	if (configIVP(ivp)
	 && MPT_metatype_convert(ivp->cfg, MPT_ENUM(TypeNodePtr), &cfg)
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
	/* trigger configuration completion */
	ret = obj->_vptr->set_property(obj, 0, 0);
	mpt_solver_info(sol, hist);
	
	/* initial output for PDE mode */
	if (!ivp->sd || ivp->sd->nval) {
		mpt_solver_status(sol, hist, 0, 0);
	}
	else {
		MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
		struct _clientPdeOut ctx;
		mpt_solver_output_query(&out, &ivp->_cfg);
		mpt_solver_output_query(&out, 0);
		ctx.state = MPT_DATASTATE(Init);
		if (ret < 0) {
			ctx.state |= MPT_DATASTATE(Fail);
		}
		ctx.out = &out;
		ctx.dat = ivp->sd;
		mpt_solver_status(sol, hist, outPDE, &ctx);
		mpt_array_clone(&out._pass, 0);
	}
	mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
	        MPT_tr("IVP client preparation finished"));
	return ret;
}
/* step operation on solver */
static int stepIVP(MPT_STRUCT(IVP) *ivp, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<IVP>::step";
	
	MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
	MPT_INTERFACE(iterator) *steps;
	MPT_INTERFACE(logger) *info, *hist;
	MPT_INTERFACE(object) *obj;
	MPT_INTERFACE(metatype) *mt;
	MPT_SOLVER(interface) *sol;
	struct _clientPdeOut ctx;
	struct rusage pre, post;
	double curr, end;
	int ret;
	
	ctx.out = &out;
	
	info = loggerIVP(0);
	
	if (!(mt = ivp->sol)
	 || !(ctx.dat = ivp->sd)) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("client not prepared for step operation"));
		return MPT_ERROR(BadOperation);
	}
	if (!(steps = args)) {
		if (mpt_config_get(&ivp->_cfg, "times", MPT_ENUM(TypeIteratorPtr), &steps) < 0
		 || !steps) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("no default time step source"));
			return MPT_ERROR(BadArgument);
		}
	}
	if ((ret = MPT_metatype_convert(mt, MPT_ENUM(TypeSolverPtr), &sol)) < 0
	 || !sol) {
		mpt_log(info, _func, MPT_LOG(Error), "%s (%d)",
		        MPT_tr("unable to get solver interface"), ret);
		return MPT_ERROR(BadOperation);
	}
	hist = loggerIVP(ivp);
	
	/* execute ODE steps */
	if (ivp->sd->nval) {
		int state = MPT_DATASTATE(Step);
		
		mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
		        MPT_tr("attempt solver steps"));
		
		/* current time data */
		getrusage(RUSAGE_SELF, &pre);
		/* execute possible steps */
		ret = mpt_solver_steps_ode((MPT_INTERFACE(convertable) *) mt, steps, hist, ivp->sd, info);
		/* add time difference */
		getrusage(RUSAGE_SELF, &post);
		mpt_timeradd_sys(&ivp->ru.sys, &pre, &post);
		mpt_timeradd_usr(&ivp->ru.usr, &pre, &post);
		
		if (!ret) {
			state |= MPT_DATASTATE(Fini);
		}
		else if (ret < 0) {
			state |= MPT_DATASTATE(Fail);
		}
		mpt_solver_output_query(&out, &ivp->_cfg);
		mpt_solver_output_query(&out, 0);
		mpt_solver_output_ode(&out, state, ivp->sd);
		mpt_array_clone(&out._pass, 0);
		
		if (!ret) {
			mpt_solver_statistics(sol, hist, &ivp->ru.usr, &ivp->ru.sys);
		}
		return ret;
	}
	obj = 0;
	if ((ret = MPT_metatype_convert(mt, MPT_ENUM(TypeObjectPtr), &obj)) < 0
	 || !obj) {
		mpt_log(info, _func, MPT_LOG(Error), "%s (%" PRIxPTR ")",
		        MPT_tr("no object interface for solver"), sol);
		return ret;
	}
	/* query solver time */
	if ((ret = getTime(sol, &curr)) < 0) {
		mpt_log(info, _func, MPT_LOG(Error), "%s (%" PRIxPTR ")",
		        MPT_tr("unable to get solver time"), sol);
		return ret;
	}
	end = curr;
	/* advance time source */
	if ((ret = nextTime(steps, &end, _func, args, info)) <= 0) {
		return ret;
	}
	mpt_solver_output_query(&out, &ivp->_cfg);
	mpt_solver_output_query(&out, 0);
	
	/* execute solver step(s) for time nodes */
	while (1) {
		ctx.state = MPT_DATASTATE(Step);
		ret = 1;
		
		mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s (t = %g > %g)",
		        MPT_tr("attempt solver step"), curr, end);
		
		/* assign solver step end time */
		if ((ret = mpt_solver_setvalue(obj, "t", end)) < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (t = %g > %g)",
			        MPT_tr("failed to set target time"), curr, end);
			break;
		}
		/* current time */
		getrusage(RUSAGE_SELF, &pre);
		
		/* execute single step */
		ret = sol->_vptr->solve(sol);
		
		/* add time difference */
		getrusage(RUSAGE_SELF, &post);
		mpt_timeradd_sys(&ivp->ru.sys, &pre, &post);
		mpt_timeradd_usr(&ivp->ru.usr, &pre, &post);
		
		/* get end time */
		if (getTime(sol, &curr) < 0) {
			mpt_log(info, _func, MPT_LOG(Warning), "%s (tend = %g)",
			        MPT_tr("solver time query failed"), end);
			if (ret >= 0) {
				end = curr;
			}
		}
		if (ret < 0) {
			ctx.state |= MPT_DATASTATE(Fail);
			mpt_solver_status(sol, info, outPDE, &ctx);
			mpt_log(info, _func, MPT_LOG(Error), "%s (t = %g)",
			        MPT_tr("solver step failed"), curr);
			break;
		}
		/* end time not reached, retry */
		if (curr < end) {
			mpt_solver_status(sol, info, outPDE, &ctx);
			continue;
		}
		end = curr;
		if ((ret = steps->_vptr->advance(steps)) < 0) {
			/* interrupt iteration */
			mpt_log(info, _func, MPT_LOG(Error), "%s (t = %g, %d, %s)",
			        MPT_tr("time source in bad state"), end, ret,
			        args ? MPT_tr("argument") : MPT_tr("internal"));
			ret = 0;
		}
		/* regular iteration termination */
		else if (!ret) {
			ctx.state |= MPT_DATASTATE(Fini);
		}
		mpt_solver_status(sol, info, outPDE, &ctx);
		
		if (!ret) {
			mpt_solver_statistics(sol, hist, &ivp->ru.usr, &ivp->ru.sys);
		}
		if (!args || ret < 1) {
			break;
		}
		if ((ret = nextTime(steps, &end, _func, args, info)) <= 0) {
			ret = 0;
			break;
		}
	}
	mpt_array_clone(&out._pass, 0);
	return ret;
}
/* client interface */
static int processIVP(MPT_INTERFACE(client) *cl, uintptr_t id, MPT_INTERFACE(iterator) *it)
{
	MPT_STRUCT(IVP) *ivp = (void *) cl;
	int ret;
	
	/* ignore empty iterator */
	if (it && !it->_vptr->value(it)) {
		it = 0;
	}
	
	if (!id || id == mpt_hash("start", 5)) {
		MPT_STRUCT(node) *cfg = 0;
		if (!configIVP(ivp)
		 || MPT_metatype_convert(ivp->cfg, MPT_ENUM(TypeNodePtr), &cfg) < 0
		 || !cfg) {
			return MPT_ERROR(BadOperation);
		}
		if (id && (ret = mpt_solver_read(cfg, it, loggerIVP(0))) < 0) {
			return ret;
		}
		if ((ret = initIVP(ivp, id ? 0 : it)) < 0) {
			return ret;
		}
		if ((ret = prepIVP(ivp, 0)) < 0) {
			return ret;
		}
		return MPT_EVENTFLAG(Default);
	}
	else if (id == mpt_hash("read", 4)) {
		MPT_STRUCT(node) *cfg = 0;
		if (!configIVP(ivp)
		 || MPT_metatype_convert(ivp->cfg, MPT_ENUM(TypeNodePtr), &cfg) < 0
		 || !cfg) {
			return MPT_ERROR(BadOperation);
		}
		ret = mpt_solver_read(cfg, it, loggerIVP(0));
	}
	else if (id == mpt_hash("init", 4)) {
		ret = initIVP(ivp, it);
	}
	else if (id == mpt_hash("prep", 4)) {
		ret = prepIVP(ivp, it);
	}
	else if (id == mpt_hash("step", 4)) {
		ret = stepIVP(ivp, it);
	}
	else if (id == mpt_hash("cont", 4)) {
		return MPT_EVENTFLAG(Default);
	}
	else if (id == mpt_hash("set", 3)) {
		ret = mpt_config_args(&ivp->_cfg, it, loggerIVP(0));
	}
	else if (id == mpt_hash("unset", 5) || id == mpt_hash("del", 3)) {
		ret = mpt_config_clear(&ivp->_cfg, it, loggerIVP(0));
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
static int dispatchIVP(MPT_INTERFACE(client) *cl, MPT_STRUCT(event) *ev)
{
	MPT_STRUCT(IVP) *ivp = (void *) cl;
	int err;
	
	if (!ev) {
		if ((err = stepIVP(ivp, 0)) < 0) {
			return err;
		}
		return err ? MPT_EVENTFLAG(Default) : MPT_EVENTFLAG(Terminate);
	}
	if (!ev->msg) {
		if ((err = stepIVP(ivp, 0)) < 0) {
			return MPT_event_fail(ev, err, MPT_tr("bad step operation"));
		}
		if (err) {
			mpt_context_reply(ev->reply, err, "%s (%" PRIxPTR ")",
			                  MPT_tr("solver step processed"), ev->id);
			return MPT_EVENTFLAG(None);
		}
		mpt_context_reply(ev->reply, err, "%s (%" PRIxPTR ")",
		                  MPT_tr("solver operation finished"), ev->id);
		ev->id = 0;
		return MPT_EVENTFLAG(Default);
	}
	return mpt_solver_dispatch(cl, ev);
}

/*!
 * \ingroup mptSolver
 * \brief IVP client creation
 * 
 * Create client for solving Initial Value Problems.
 * 
 * \param uinit user initialization function
 * \param base  global config path (dot delimited)
 * 
 * \return IVP client
 */
extern MPT_INTERFACE(client) *mpt_client_ivp(MPT_SOLVER_TYPE(UserInit) uinit)
{
	static const MPT_INTERFACE_VPTR(config) configIVP = {
		queryIVP, assignIVP, removeIVP
	};
	static const MPT_INTERFACE_VPTR(client) clientIVP = {
		{ { convIVP }, deleteIVP, addrefIVP, cloneIVP },
		dispatchIVP,
		processIVP
	};
	MPT_STRUCT(IVP) *ivp;
	
	if (!uinit) {
		return 0;
	}
	if (!(ivp = malloc(sizeof(*ivp)))) {
		return 0;
	}
	ivp->_cl._vptr = &clientIVP;
	ivp->_cfg._vptr = &configIVP;
	
	ivp->sd = 0;
	ivp->cfg = 0;
	ivp->sol = 0;
	ivp->uinit = uinit;
	
	memset(&ivp->ru, 0, sizeof(ivp->ru));
	
	return &ivp->_cl;
}

