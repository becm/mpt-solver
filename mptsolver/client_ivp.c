/*!
 * create client for IVP problem types
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <sys/resource.h>
#include <sys/uio.h>

#include "node.h"
#include "message.h"
#include "array.h"
#include "meta.h"

#include "values.h"
#include "output.h"
#include "parse.h"

#include "client.h"
#include "config.h"

#include "solver.h"

MPT_STRUCT(IVP) {
	MPT_INTERFACE(client) _cl;
	MPT_INTERFACE(config) _cfg;
	
	MPT_STRUCT(proxy) pr;
	
	MPT_STRUCT(solver_output) out;
	
	MPT_INTERFACE(metatype) *steps;
	
	MPT_STRUCT(solver_data) *sd;
	
	MPT_INTERFACE(metatype) *cfg;
	
	MPT_SOLVER(interface) *sol;
	
	MPT_SOLVER_TYPE(UserInit) *uinit;
	
	struct timeval ru_usr,   /* user time in solver backend */
	               ru_sys;   /* system time in solver backend */
	double t;
	int pdim;
};

static int setTime(void *ptr, const MPT_STRUCT(property) *pr)
{
	if (!pr || pr->name) {
		return 0;
	}
	if (!pr->val.fmt || *pr->val.fmt != 'd') {
		return MPT_ERROR(BadValue);
	}
	*((double *) ptr) = *((double *) pr->val.ptr);
	return 1;
}

static double getTime(MPT_SOLVER(interface) *sol, double t)
{
	sol->_vptr->report(sol, MPT_SOLVER_ENUM(Values), setTime, &t);
	return t;
}

static int nextTime(MPT_INTERFACE(iterator) *src, double *end, const char *fcn, void *arg, MPT_INTERFACE(logger) *info)
{
	double ref = *end;
	
	while (1) {
		double next;
		int ret, adv;
		if ((ret = src->_vptr->get(src, 'd', &next)) < 0) {
			mpt_log(info, fcn, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("bad value on time source"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"));
			return ret;
		}
		if (!ret) {
			mpt_log(info, fcn, MPT_LOG(Info), "%s (%s)",
			        MPT_tr("no further data in time source"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"));
			return 0;
		}
		if (ref < next) {
			*end = next;
			return ret;
		}
		mpt_log(info, fcn, MPT_LOG(Info), "%s (%g <= %g)",
		        MPT_tr("skip time value argument"), next, ref);
		
		if ((adv = src->_vptr->advance(src)) < 0) {
			mpt_log(info, fcn, MPT_LOG(Error), "%s (%s)",
			        MPT_tr("bad time source state"),
			        arg ? MPT_tr("argument") : MPT_tr("internal"));
			return adv;
		}
		if (!adv) {
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
static MPT_STRUCT(node) *configIVP(MPT_STRUCT(IVP) *ivp)
{
	MPT_INTERFACE(metatype) *cfg;
	MPT_STRUCT(node) *n;
	
	if (!(cfg = ivp->cfg)) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		mpt_path_set(&p, "mpt.config", -1);
		if (!(cfg = mpt_config_global(&p))) {
			return 0;
		}
		ivp->cfg = cfg;
	}
	n = 0;
	if (cfg->_vptr->conv(cfg, MPT_ENUM(TypeNode), &n) < 0) {
		return 0;
	}
	return n;
}
static const MPT_INTERFACE(metatype) *queryIVP(const MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	MPT_STRUCT(IVP) *ivp = MPT_baseaddr(IVP, gen, _cfg);
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!(conf = configIVP(ivp))) {
		return 0;
	}
	if (!porg) {
		return ivp->pr._ref;
	}
	if (!porg->len) {
		return conf->_meta;
	}
	if (!(conf = conf->children)) {
		return 0;
	}
	p = *porg;
	p.flags &= ~MPT_PATHFLAG(HasArray);
	
	if (!(conf = mpt_node_query(conf, &p, 0))) {
		return 0;
	}
	if (!conf->_meta) {
		return mpt_metatype_default();
	}
	return conf->_meta;
}
static int assignIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val)
{
	static const char _func[] = "mpt::client<IVP>::assign";
	MPT_STRUCT(IVP) *ivp = MPT_baseaddr(IVP, gen, _cfg);
	MPT_INTERFACE(logger) *info;
	MPT_STRUCT(node) *conf;
	
	if (!(info = mpt_solver_output_logger(&ivp->out))) {
		info = mpt_log_default();
	}
	if (!porg) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		if (ivp->cfg) {
			return MPT_ERROR(BadOperation);
		}
		if (!val) {
			return configIVP(ivp) ? 0 : MPT_ERROR(BadOperation);
		}
		if (val->fmt) {
			return MPT_ERROR(BadType);
		}
		if (!val->ptr
		    || !mpt_path_set(&p, val->ptr, -1)) {
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
		int ret = mpt_node_parse(conf, val, info);
		if (ret >= 0) {
			mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
			        MPT_tr("loaded IVP client config file"));
		}
		return ret;
	}
	if (!(conf = mpt_node_assign(&conf->children, porg, val))) {
		mpt_log(info, _func, MPT_LOG(Critical), "%s",
		        MPT_tr("unable to assign client element"));
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
static int removeIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	static const char _func[] = "mpt::client<IVP>::remove";
	
	MPT_STRUCT(IVP) *ivp = MPT_baseaddr(IVP, gen, _cfg);
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		MPT_INTERFACE(metatype) *mt;
		MPT_INTERFACE(object) *obj;
		
		/* close history output */
		if ((mt = ivp->out._data)
		    && mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
		    && obj
		    && mpt_conf_history(obj, 0) < 0) {
			MPT_INTERFACE(logger) *info = mpt_solver_output_logger(&ivp->out);
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("unable to close history output"));
		}
		if (ivp->sd) {
			mpt_solver_data_clear(ivp->sd);
		}
		if ((mt = ivp->pr._ref)) {
			mt->_vptr->ref.unref((void *) mt);
			ivp->pr._ref = 0;
			ivp->pr._hash = 0;
			
			ivp->sol = 0;
			return 1;
		}
		return 0;
	}
	if (!(conf = configIVP(ivp))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg->len) {
		mpt_node_clear(conf);
		return 1;
	}
	p = *porg;
	p.flags &= ~MPT_PATHFLAG(HasArray);
	
	if (!(conf = mpt_node_query(conf->children, &p, 0))) {
		return MPT_ERROR(BadArgument);
	}
	mpt_node_clear(conf);
	return 1;
}
/* reference interface */
static void deleteIVP(MPT_INTERFACE(reference) *gen)
{
	MPT_STRUCT(IVP) *ivp = (void *) gen;
	MPT_INTERFACE(metatype) *it;
	
	mpt_proxy_fini(&ivp->pr);
	mpt_solver_output_close(&ivp->out);
	
	if ((it = ivp->steps)) {
		it->_vptr->ref.unref((void *) it);
	}
	if (ivp->sd) {
		mpt_solver_data_fini(ivp->sd);
		free(ivp->sd);
	}
	if (ivp->cfg) {
		free(ivp->cfg);
	}
	free(ivp);
}
static uintptr_t addrefIVP(MPT_INTERFACE(reference) *gen)
{
	(void) gen;
	return 0;
}
/* metatype interface */
static int convIVP(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(IVP) *ivp = (void *) mt;
	int me = mpt_client_typeid();
	
	if (me < 0) {
		me = MPT_ENUM(TypeMeta);
	}
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeConfig), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return me;
	}
	if (type == MPT_ENUM(TypeConfig)) {
		if (ptr) *((const void **) ptr) = &ivp->_cfg;
		return me;
	}
	if (type == me) {
		if (ptr) *((const void **) ptr) = &ivp->_cl;
		return MPT_ENUM(TypeConfig);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *cloneIVP(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* init operation for solver */
static int initIVP(MPT_INTERFACE(client) *cl, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<IVP>::init";
	
	MPT_STRUCT(IVP) *ivp = (void *) cl;
	MPT_STRUCT(node) *conf, *curr;
	MPT_STRUCT(solver_data) *dat;
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(object) *obj;
	MPT_INTERFACE(logger) *info;
	const char *val;
	int ret;
	
	if (!(info = mpt_solver_output_logger(&ivp->out))) {
		info = mpt_log_default();
	}
	
	if (!(conf = configIVP(ivp))) {
		mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to query"), MPT_tr("client configuration"));
		return MPT_ERROR(BadOperation);
	}
	/* reevaluate solver config file */
	if ((curr = mpt_node_find(conf, "solconf", 1))
	    && !curr->children
	    && (val = mpt_node_data(curr, 0))) {
		MPT_STRUCT(parse) parse = MPT_PARSE_INIT;
		if (!(parse.src.arg = fopen(val, "r"))) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s: %s",
			        MPT_tr("failed to open"), MPT_tr("solver config"), val);
			return MPT_ERROR(BadOperation);
		}
		parse.src.getc = (int (*)()) mpt_getchar_stdio;
		ret = mpt_parse_node(curr, &parse, "[ ] = !#");
		fclose(parse.src.arg);
		if (ret < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (%d): line = %d: %s",
			        MPT_tr("parse error"), parse.curr, (int) parse.src.line, val);
			return ret;
		}
		mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s: %s",
		        MPT_tr("loaded solver config file"), val);
	}
	/* use supplied initial value */
	if (args) {
		if ((mt = ivp->steps)) {
			mt->_vptr->ref.unref((void *) mt);
			ivp->steps = 0;
		}
	}
	/* reset existing time source */
	else if (!(curr = mpt_node_find(conf, "times", 1))) {
		if ((mt = ivp->steps)
		    && (ret = mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &args)) >= 0
		    && args
		    && (ret = args->_vptr->reset(args)) < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("bad time source"), MPT_tr("unable to reset"));
			return ret;
		}
	}
	/* create time step source */
	else {
		MPT_INTERFACE(metatype) *old;
		val = mpt_node_data(curr, 0);
		
		if (!val) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("missing time source"));
			return MPT_ERROR(MissingData);
		}
		if (!(mt = mpt_iterator_create(val))) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("bad iteratior description"), val);
			return MPT_ERROR(BadValue);
		}
		if ((ret = mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &args)) < 0) {
			mt->_vptr->ref.unref((void *) mt);
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("bad iteratior metatype"), val);
			return MPT_ERROR(BadType);
		}
		if ((old = ivp->steps)) {
			old->_vptr->ref.unref((void *) old);
		}
		ivp->steps = mt;
	}
	ivp->t = 0;
	if (args) {
		if ((ret = args->_vptr->get(args, 'd', &ivp->t)) <= 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("failed to query"), MPT_tr("initial time value"));
			return ret;
		}
		if (ret && (ret = args->_vptr->advance(args)) < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
			        MPT_tr("unable to advance iterator"), MPT_tr("time steps"));
		}
	}
	/* clear/create solver data */
	if ((dat = ivp->sd)) {
		mpt_solver_data_clear(dat);
	}
	else if ((dat = malloc(sizeof(*dat)))) {
		const MPT_STRUCT(solver_data) sd = MPT_SOLVER_DATA_INIT;
		*dat = sd;
		ivp->sd = dat;
	}
	else {
		mpt_log(info, _func, MPT_LOG(Error), "%s: %s",
		        MPT_tr("failed to allocate memory"), MPT_tr("solver data"));
		return MPT_ERROR(BadOperation);
	}
	/* get PDE grid data */
	if ((curr = mpt_node_next(conf->children, "grid"))) {
		if ((ret = mpt_conf_grid(&dat->val, curr->_meta)) < 0) {
			mpt_log(info, _func, MPT_LOG(Warning), "%s: %s",
			        "grid", MPT_tr("invalid parameter format"));
		}
		dat->nval = ret;
	}
	/* setup ODE mode */
	else {
		/* save initial time value */
		if (!mpt_array_append(&dat->val, sizeof(ivp->t), &ivp->t)) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("unable to save initial time in history"));
			return MPT_ERROR(BadOperation);
		}
		/* add profile values */
		if ((curr = mpt_node_next(conf->children, "profile"))
		    && (ret = mpt_conf_param(&dat->val, curr, 1)) < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("failed to reserve initial data"));
			return ret;
		}
	}
	/* get user parameter */
	if ((curr = conf->children)
	    && (curr = mpt_node_next(curr, "param"))) {
		if ((ret = mpt_conf_param(&dat->param, curr, 0)) < 0) {
			mpt_log(info, _func, MPT_LOG(Warning), "%s: %s",
			        "param", MPT_tr("invalid parameter format"));
		}
		dat->npar = ret;
	}
	if ((mt = ivp->out._graphic)) {
		MPT_INTERFACE(output) *out = 0;
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &out) >= 0
		    && out) {
			mpt_conf_graphic(out, conf, info);
		}
	}
	/* load new solver or evaluate existing */
	curr = mpt_node_find(conf, "solver", 1);
	val = curr ? mpt_node_data(curr, 0) : 0;
	ivp->sol = mpt_solver_load(&ivp->pr, MPT_SOLVER_ENUM(CapableIvp), val, info);
	
	obj = 0;
	if (!(mt = (void *) ivp->sol)
	    || mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) < 0
	    || !obj) {
		return MPT_ERROR(BadValue);
	}
	val = mpt_object_typename(obj);
	if (val) {
		mpt_log(info, 0, MPT_LOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	
	if ((ret = ivp->uinit(ivp->sol, dat, info)) < 0) {
		return ret;
	}
	ivp->pdim = dat->nval;
	/* set data segment size */
	dat->nval = dat->val._buf->_used / sizeof(double);
	
	/* setup PDE time and profile data */
	if (ivp->pdim) {
		MPT_STRUCT(node) *pcfg = mpt_node_next(conf->children, "profile");
		int err;
		/* no profile data requested or available */
		if (!ret || !pcfg) {
			if ((err = mpt_solver_setvalue(obj, 0, ivp->t)) < 0) {
				mpt_log(info, 0, MPT_LOG(Error), "%s: %s",
				        MPT_tr("solver"), MPT_tr("failed to set initial time"));
				return err;
			}
		}
		/* process profile data */
		else if (!(mt = mpt_conf_profiles(dat, ivp->t, pcfg, ret, info))) {
			return MPT_ERROR(BadOperation);
		}
		else {
			err = obj->_vptr->setProperty(obj, 0, mt);
			mt->_vptr->ref.unref((void *) mt);
			if (err < 0) {
				mpt_log(info, _func, MPT_LOG(Error), "%s",
				        MPT_tr("PDE init state assignment failed"));
				return err;
			}
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
		if ((ret = mpt_init_ivp(obj, &dat->val, info)) < 0) {
			return ret;
		}
	}
	mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
	        MPT_tr("IVP client preparation finished"));
	
	return 0;
}
/* step operation on solver */
static int stepIVP(MPT_INTERFACE(client) *cl, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<IVP>::step";
	
	MPT_STRUCT(IVP) *ivp = (void *) cl;
	MPT_INTERFACE(iterator) *steps;
	MPT_INTERFACE(logger) *info;
	MPT_INTERFACE(object) *obj;
	MPT_SOLVER(interface) *sol;
	struct _clientPdeOut ctx;
	struct rusage pre, post;
	double end;
	int ret;
	
	if (!(info = mpt_solver_output_logger(&ivp->out))) {
		info = mpt_log_default();
	}
	ctx.out = &ivp->out;
	
	if (!(sol = ivp->sol) || !(ctx.dat = ivp->sd)) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("client not prepared for step operation"));
		return MPT_ERROR(BadOperation);
	}
	if (!(steps = args)) {
		MPT_INTERFACE(metatype) *src;
		if (!(src = ivp->steps)
		    || (ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &steps)) < 0
		    || !steps) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("no time step source"));
			return MPT_ERROR(BadArgument);
		}
	}
	/* execute ODE steps */
	if (!ivp->pdim) {
		int state = MPT_DATASTATE(Step);
		
		/* current time data */
		getrusage(RUSAGE_SELF, &pre);
		/* execute possible steps */
		ret = mpt_steps_ode(sol, steps, ivp->sd, info);
		/* add time difference */
		getrusage(RUSAGE_SELF, &post);
		mpt_timeradd_sys(&ivp->ru_sys, &pre, &post);
		mpt_timeradd_usr(&ivp->ru_usr, &pre, &post);
		
		if (!ret) {
			state |= MPT_DATASTATE(Fini);
		}
		else if (ret < 0) {
			state |= MPT_DATASTATE(Fail);
		}
		mpt_solver_output_ode(&ivp->out, state, ivp->sd);
		
		if (!ret) {
			mpt_solver_statistics(sol, info, &ivp->ru_usr, &ivp->ru_sys);
		}
		return ret;
	}
	obj = 0;
	if ((ret = sol->_vptr->meta.conv((void *) sol, MPT_ENUM(TypeObject), &obj)) < 0
	    || !obj) {
		mpt_log(info, _func, MPT_LOG(Error), "%s (" PRIxPTR ")",
		        MPT_tr("no object interface for solver"), sol);
		return ret;
	}
	/* advance time source */
	end = ivp->t;
	if ((ret = nextTime(steps, &end, _func, args, info)) <= 0) {
		return ret;
	}
	/* execute solver step(s) for time nodes */
	while (1) {
		ctx.state = MPT_DATASTATE(Step);
		ret = 1;
		
		getrusage(RUSAGE_SELF, &pre);
		
		mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s (t = %g > %g)",
		        MPT_tr("attempt solver step"), ivp->t, end);
		
		/* assign solver step end time */
		if ((ret = mpt_solver_setvalue(obj, "t", end)) < 0) {
			mpt_log(info, __func__, MPT_LOG(Debug2), "%s (t = %g > %g)",
			        MPT_tr("failed to set target time"), ivp->t, end);
			return ret;
		}
		ret = sol->_vptr->solve(sol);
		
		/* collect time difference */
		getrusage(RUSAGE_SELF, &post);
		mpt_timeradd_sys(&ivp->ru_sys, &pre, &post);
		mpt_timeradd_usr(&ivp->ru_usr, &pre, &post);
		
		/* get end time */
		ivp->t = getTime(sol, end);
		if (ret < 0) {
			ctx.state |= MPT_DATASTATE(Fail);
			mpt_log(info, _func, MPT_LOG(Error), "%s (t = %g)",
			        MPT_tr("solver step failed"), end);
			return ret;
		}
		/* end time not reached, retry */
		if (ivp->t < end) {
			mpt_solver_status(sol, info, outPDE, &ctx);
			continue;
		}
		end = ivp->t;
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
			mpt_solver_statistics(sol, info, &ivp->ru_usr, &ivp->ru_sys);
		}
		if (!args || ret < 1) {
			return ret;
		}
		if ((ret = nextTime(steps, &end, _func, args, info)) <= 0) {
			return ret;
		}
	}
}
/* client interface */
static int processIVP(MPT_INTERFACE(client) *cl, uintptr_t id, MPT_INTERFACE(iterator) *it)
{
	static const char _func[] = "mpt::client<IVP>::process";
	
	MPT_STRUCT(IVP) *ivp = (void *) cl;
	MPT_INTERFACE(logger) *info;
	int ret;
	
	if (!(info = mpt_solver_output_logger(&ivp->out))) {
		info = mpt_log_default();
	}
	
	if (!id) {
		const MPT_INTERFACE(metatype) *mt;
		MPT_INTERFACE(object) *obj = 0;
		if (!(mt = ivp->pr._ref)
		    || mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) < 0
		    || !obj) {
			return MPT_EVENTFLAG(Fail);
		}
		/* TODO: set solver parameters from args */
		ret = obj->_vptr->setProperty(obj, 0, 0);
	}
	else if (id == mpt_hash("init", 4)) {
		ret = initIVP(cl, it);
	}
	else if (id == mpt_hash("step", 4)) {
		ret = stepIVP(cl, it);
	}
	else if (id == mpt_hash("set", 3)) {
		ret = mpt_config_args(&ivp->_cfg, it);
		if (ret > 0) {
			const char *val = "";
			if (it) {
				it->_vptr->get(it, 's', &val);
			}
			mpt_log(info, _func, MPT_LOG(Error), "%s (%d): %s",
			        MPT_tr("bad assign argument"), ret, val);
			return MPT_EVENTFLAG(Fail);
		}
	}
	else if (id == mpt_hash("unset", 5) || id == mpt_hash("del", 3)) {
		ret = mpt_config_clear(&ivp->_cfg, it, info);
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
	if (!ev) {
		return 0;
	}
	if (!ev->msg) {
		return stepIVP(cl, 0);
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
extern MPT_INTERFACE(client) *mpt_client_ivp(MPT_SOLVER_TYPE(UserInit) uinit, MPT_INTERFACE(metatype) *out)
{
	static const MPT_INTERFACE_VPTR(config) configIVP = {
		queryIVP, assignIVP, removeIVP
	};
	static const MPT_INTERFACE_VPTR(client) clientIVP = {
		{ { deleteIVP, addrefIVP }, convIVP, cloneIVP },
		dispatchIVP,
		processIVP
	};
	const MPT_STRUCT(solver_output) def = MPT_SOLVER_OUTPUT_INIT;
	MPT_STRUCT(IVP) *ivp;
	
	if (!uinit || !out) {
		return 0;
	}
	if (!(ivp = malloc(sizeof(*ivp)))) {
		return 0;
	}
	ivp->_cl._vptr = &clientIVP;
	ivp->_cfg._vptr = &configIVP;
	
	memset(&ivp->pr, 0, sizeof(ivp->pr));
	ivp->sd = 0;
	ivp->cfg = 0;
	ivp->steps = 0;
	
	ivp->out = def;
	ivp->out._data = out;
	
	ivp->sol = 0;
	ivp->uinit = uinit;
	memset(&ivp->ru_usr, 0, sizeof(ivp->ru_usr));
	memset(&ivp->ru_sys, 0, sizeof(ivp->ru_sys));
	ivp->t = 0.0;
	ivp->pdim = 0;
	
	return &ivp->_cl;
}

