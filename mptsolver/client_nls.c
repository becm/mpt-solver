/*!
 * create client for solving nonlinear systems.
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

MPT_STRUCT(NLS) {
	MPT_INTERFACE(client) _cl;
	MPT_INTERFACE(config) _cfg;
	
	MPT_STRUCT(proxy) pr;
	
	MPT_STRUCT(solver_data) *sd;
	
	MPT_INTERFACE(metatype) *cfg;
	
	MPT_SOLVER(interface) *sol;
	
	MPT_SOLVER_TYPE(UserInit) *uinit;
	
	struct timeval  ru_usr,  /* user time in solver backend */
	                ru_sys;  /* system time in solver backend */
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
		return mpt_solver_output_nls(ctx->out, ctx->state, val, ctx->dat);
	}
	return ret;
}
static MPT_INTERFACE(logger) *loggerNLS(const MPT_STRUCT(NLS) *nls)
{
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *info;
	
	if ((mt = nls->cfg)) {
		MPT_INTERFACE(config) *cfg = 0;
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeConfig), &cfg) > 0
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

static MPT_STRUCT(node) *configNLS(MPT_STRUCT(NLS) *nls)
{
	MPT_INTERFACE(metatype) *cfg;
	MPT_STRUCT(node) *n;
	
	if (!(cfg = nls->cfg)) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		mpt_path_set(&p, "mpt.config", -1);
		if (!(cfg = mpt_config_global(&p))) {
			return 0;
		}
		nls->cfg = cfg;
	}
	n = 0;
	if (cfg->_vptr->conv(cfg, MPT_ENUM(TypeNode), &n) < 0) {
		return 0;
	}
	return n;
}
/* config interface */
static const MPT_INTERFACE(metatype) *queryNLS(const MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		return nls->pr._ref;
	}
	if (!(conf = configNLS(nls))) {
		return 0;
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
static int assignNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val)
{
	static const char _func[] = "mpt::client<NLS>::assign";
	
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_STRUCT(node) *conf;
	
	if (!porg) {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		if (nls->cfg) {
			return MPT_ERROR(BadOperation);
		}
		if (!val) {
			return configNLS(nls) ? 0 : MPT_ERROR(BadOperation);
		}
		if (val->fmt) {
			return MPT_ERROR(BadType);
		}
		if (!val->ptr
		    || !mpt_path_set(&p, val->ptr, -1)) {
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
		MPT_INTERFACE(logger) *info = loggerNLS(nls);
		int ret = mpt_node_parse(conf, val, info);
		if (ret >= 0) {
			mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s",
			        MPT_tr("loaded NLS client config file"));
		}
		return ret;
	}
	if (!(conf = mpt_node_assign(&conf->children, porg, val))) {
		mpt_log(loggerNLS(nls), _func, MPT_LOG(Critical), "%s",
		        MPT_tr("unable to assign client element"));
		return MPT_ERROR(BadOperation);
	}
	return 0;
}
static int removeNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		if (nls->sd) {
			mpt_solver_data_clear(nls->sd);
		}
		if (nls->pr._ref) {
			return 0;
		}
		mpt_proxy_fini(&nls->pr);
		nls->sol = 0;
		return 1;
	}
	if (!(conf = configNLS(nls))) {
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
static void deleteNLS(MPT_INTERFACE(reference) *gen)
{
	MPT_STRUCT(NLS) *nls = (void *) gen;
	MPT_INTERFACE(metatype) *mt;
	
	mpt_proxy_fini(&nls->pr);
	
	if (nls->sd) {
		mpt_solver_data_fini(nls->sd);
		free(nls->sd);
	}
	if ((mt = nls->cfg)) {
		mt->_vptr->ref.unref((void *) mt);
	}
	free(nls);
}
static uintptr_t addrefNLS(MPT_INTERFACE(reference) *gen)
{
	(void) gen;
	return 0;
}
/* metatype interface */
static int convNLS(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(NLS) *nls = (void *) mt;
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
		if (ptr) *((const void **) ptr) = &nls->_cfg;
		return me;
	}
	if (type == me) {
		if (ptr) *((const void **) ptr) = &nls->_cl;
		return MPT_ENUM(TypeConfig);
	}
	return MPT_ERROR(BadType);
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
	MPT_INTERFACE(logger) *info;
	const char *val;
	int ret;
	
	(void) args;
	
	info = loggerNLS(nls);
	
	if (!(conf = configNLS(nls))) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("unable to get NLS client config"));
		return MPT_ERROR(BadOperation);
	}
	if ((curr = mpt_node_find(conf, "output", 1))) {
		MPT_INTERFACE(metatype) *old;
		mt = mpt_output_local();
		mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj);
		mpt_conf_history(obj, curr);
		if ((old = curr->_meta)) {
			old->_vptr->ref.unref((void *) old);
		}
		curr->_meta = mt;
		info = loggerNLS(nls);
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
	/* rebind graphic associations */
	if ((curr = conf->children)) {
		MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
		mpt_solver_output_query(&out, &nls->_cfg);
		mpt_solver_output_query(&out, 0);
		if (out._graphic) {
			mpt_conf_graphic(out._graphic, conf->children, info);
		}
		mpt_array_clone(&out._pass, 0);
	}
	if (!(mt = (void *) nls->sol)) {
		if (!(mt = nls->pr._ref)) {
			curr = mpt_node_find(conf, "solver", 1);
			val = curr ? mpt_node_data(curr, 0) : 0;
			if (!(nls->sol = mpt_solver_load(&nls->pr, MPT_SOLVER_ENUM(CapableNls), val, info))) {
				return MPT_ERROR(BadValue);
			}
			mt = (void *) nls->sol;
		}
		else if ((mt->_vptr->conv(mt, mpt_solver_typeid(), &nls->sol) < 0
		         || !nls->sol)) {
			int type = mt->_vptr->conv(mt, 0, 0);
			mpt_log(info, _func, MPT_LOG(Error), "%s: 0x%x",
			        MPT_tr("invalid type in solver"), type);
			return MPT_ERROR(BadType);
		}
	}
	val = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) > 0
	    && obj) {
		val = mpt_object_typename(obj);
	}
	if (val) {
		mpt_log(info, 0, MPT_LOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	if ((ret = nls->uinit(nls->sol, nls->sd, info)) < 0) {
		return ret;
	}
	return 0;
}
/* prepare operation on solver */
static int prepNLS(MPT_STRUCT(NLS) *nls, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<NLS>::prep";
	
	const MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(object) *obj;
	MPT_STRUCT(node) *cfg;
	int err;
	
	if (!(mt = nls->pr._ref)) {
		mpt_log(loggerNLS(nls), _func, MPT_LOG(Warning), "%s (%" PRIxPTR ")",
		        MPT_tr("missing solver interface"), nls);
		return MPT_ERROR(BadOperation);
	}
	obj = 0;
	if (mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) < 0
	    || !obj) {
		err = mt->_vptr->conv(mt, 0, 0);
		mpt_log(loggerNLS(nls), _func, MPT_LOG(Warning), "%s (%s = %d)",
		        MPT_tr("solver without object interface"), MPT_tr("type"), err);
		return MPT_ERROR(BadOperation);
	}
	/* set solver parameters from config */
	if ((cfg = configNLS(nls))
	    && (cfg = cfg->children)) {
		mpt_solver_param(obj, cfg, loggerNLS(nls));
	}
	/* set solver parameters from args */
	if (args && (err = mpt_object_args(obj, args)) < 0) {
		return err;
	}
	return obj->_vptr->setProperty(obj, 0, 0);
}
/* step operation for solver */
static int stepNLS(MPT_STRUCT(NLS) *nls, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<NLS>::step";
	
	MPT_STRUCT(solver_data) *dat;
	MPT_SOLVER(interface) *sol;
	MPT_STRUCT(node) *names;
	struct _outNLSdata ctx;
	struct rusage pre, post;
	const double *par;
	MPT_STRUCT(solver_output) out = MPT_SOLVER_OUTPUT_INIT;
	MPT_INTERFACE(logger) *info;
	int res;
	
	(void) args;
	
	info = loggerNLS(nls);
	
	if (!(sol = nls->sol) || !(dat = nls->sd)) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("solver or data missing"));
		return MPT_ERROR(BadArgument);
	}
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	
	/* trigger solver iteration */
	res = sol->_vptr->solve(sol);
	
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_timeradd_sys(&nls->ru_sys, &pre, &post);
	mpt_timeradd_usr(&nls->ru_usr, &pre, &post);
	
	if (!info) {
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
	mpt_solver_status(sol, info, outNLS, &ctx);
	mpt_array_clone(&out._pass, 0);
	
	/* no final output for state */
	if (res > 0) {
		return res;
	}
	
	if ((names = configNLS(nls))
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
					mpt_log(info, 0, MPT_LOG(Message), "%s %2d: %16g (%s)",
					        desc, i+1, par[i], name);
					continue;
				}
			}
			mpt_log(info, 0, MPT_LOG(Message), "%s %2d: %16g",
			        desc, i + 1, par[i]);
		}
	}
	mpt_solver_statistics(sol, info, &nls->ru_usr, &nls->ru_sys);
	
	return res;
}
/* client interface */
static int processNLS(MPT_INTERFACE(client) *cl, uintptr_t id, MPT_INTERFACE(iterator) *it)
{
	static const char _func[] = "mpt::client<NLS>::process";
	
	MPT_STRUCT(NLS) *nls = (void *) cl;
	MPT_INTERFACE(logger) *info;
	int ret;
	
	info = loggerNLS(nls);
	
	if (!id || id == mpt_hash("start", 5)) {
		MPT_STRUCT(node) *cfg;
		if (!(cfg = configNLS(nls))) {
			return MPT_ERROR(BadOperation);
		}
		if ((ret = mpt_solver_read(cfg, id ? it : 0, info)) < 0) {
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
	if (id == mpt_hash("init", 4)) {
		ret = initNLS(nls, it);
	}
	else if (id == mpt_hash("step", 4)) {
		ret = stepNLS(nls, it);
	}
	else if (id == mpt_hash("set", 3)) {
		ret = mpt_config_args(&nls->_cfg, it);
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
		ret = mpt_config_clear(&nls->_cfg, it, info);
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
		{ { deleteNLS, addrefNLS }, convNLS, cloneNLS },
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
	
	(void) memset(&nls->pr, 0, sizeof(nls->pr));
	nls->sd = 0;
	nls->cfg = 0;
	
	nls->sol = 0;
	nls->uinit = uinit;
	memset(&nls->ru_usr, 0, sizeof(nls->ru_usr));
	memset(&nls->ru_sys, 0, sizeof(nls->ru_sys));
	
	return &nls->_cl;
}

