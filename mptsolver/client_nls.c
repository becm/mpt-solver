/*!
 * create client for solving nonlinear systems.
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <stdlib.h>
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
#include "config.h"
#include "parse.h"

#include "client.h"

#include "solver.h"

MPT_STRUCT(NLS) {
	MPT_INTERFACE(client) _cl;
	MPT_INTERFACE(config) _cfg;
	
	MPT_STRUCT(proxy) pr;
	
	MPT_STRUCT(solver_output) out;
	
	MPT_INTERFACE(metatype) *local, *remote;
	
	MPT_STRUCT(solver_data) *sd;
	char *cfg;
	
	MPT_SOLVER(interface) *sol;
	int (*uinit)(MPT_SOLVER(interface) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *);
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

static MPT_STRUCT(node) *configNLS(const char *base)
{
	if (!base) {
		return mpt_config_node(0);
	}
	else {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		MPT_STRUCT(node) *conf;
		
		mpt_path_set(&p, base, -1);
		if ((conf = mpt_config_node(&p))) {
			return conf;
		}
		if (mpt_config_set(0, base, 0, '.', 0) < 0) {
			return 0;
		}
		return mpt_config_node(base ? &p : 0);
	}
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
	if (!(conf = configNLS(nls->cfg))) {
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
	MPT_INTERFACE(logger) *info;
	
	if (!(info = mpt_solver_output_logger(&nls->out))) {
		info = mpt_log_default();
	}
	if (!(conf = configNLS(nls->cfg))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg) {
		struct _outNLSdata ctx;
		MPT_STRUCT(solver_data) *dat;
		MPT_SOLVER(interface) *sol;
		MPT_INTERFACE(metatype) *mt;
		MPT_INTERFACE(object) *obj;
		int ret;
		
		/* set (new) config base */
		if (val) {
			char *base;
			if (val->fmt || !val->ptr) {
				return MPT_ERROR(BadValue);
			}
			/* query config base */
			if (!configNLS(val->ptr)) {
				return MPT_ERROR(BadValue);
			}
			if (!(base = strdup(val->ptr))) {
				return MPT_ERROR(BadOperation);
			}
			if (nls->cfg) {
				free(nls->cfg);
			}
			nls->cfg = base;
			
			return strlen(base);
		}
		if (!(dat = nls->sd)) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("missing data descriptor"));
			return MPT_ERROR(BadOperation);
		}
		if (!(sol = nls->sol)) {
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("missing solver descriptor"));
			return MPT_ERROR(BadOperation);
		}
		/* set solver parameters from config */
		mpt_solver_param((void *) sol, conf->children, info);
		
		obj = 0;
		if ((mt = nls->out._data)
		    && mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
		    && obj) {
			if ((ret = mpt_conf_history(obj, conf->children)) < 0) {
				return ret;
			}
		}
		if (info) {
			mpt_solver_info((void *) sol, info);
			mpt_log(info, 0, MPT_LOG(Message), "");
		}
		ctx.out = &nls->out;
		ctx.dat = dat;
		ctx.state = MPT_DATASTATE(Init);
		
		mpt_solver_status((void *) sol, info, outNLS, &ctx);
		
		return 0;
	}
	if (!porg->len) {
		int ret = mpt_node_parse(conf, val, info);
		if (ret >= 0) {
			mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s: %s",
			        MPT_tr("loaded NLS client config file"));
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
static int removeNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	static const char _func[] = "mpt::client<NLS>::remove";
	
	MPT_STRUCT(NLS) *nls = MPT_baseaddr(NLS, gen, _cfg);
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		MPT_INTERFACE(metatype) *mt;
		MPT_INTERFACE(object) *obj;
		
		/* close history output */
		if ((mt = nls->out._data)
		    && mt->_vptr->conv(mt, MPT_ENUM(TypeObject), &obj) >= 0
		    && obj
		    && mpt_conf_history(obj, 0) < 0) {
			MPT_INTERFACE(logger) *info = mpt_solver_output_logger(&nls->out);
			mpt_log(info, _func, MPT_LOG(Error), "%s",
			        MPT_tr("unable to close history output"));
		}
		if (nls->sd) {
			mpt_solver_data_clear(nls->sd);
		}
		if ((mt = nls->pr._ref)) {
			mt->_vptr->ref.unref((void *) mt);
			nls->pr._ref = 0;
			nls->pr._hash = 0;
			
			nls->sol = 0;
			return 1;
		}
		return 0;
	}
	if (!(conf = configNLS(nls->cfg))) {
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
	
	mpt_proxy_fini(&nls->pr);
	mpt_solver_output_close(&nls->out);
	
	if (nls->sd) {
		mpt_solver_data_fini(nls->sd);
		free(nls->sd);
	}
	if (nls->cfg) {
		free(nls->cfg);
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
/* client interface */
static int initNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(iterator) *args)
{
	static const char _func[] = "mpt::client<NLS>::init";
	
	MPT_STRUCT(NLS) *nls = (void *) cl;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(solver_data) *dat;
	MPT_STRUCT(node) *conf, *sol;
	MPT_INTERFACE(logger) *info = 0;
	const char *val;
	int ret;
	
	(void) args;
	
	if (!(mt = nls->out._info)
	    || mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &info) < 0
	    || !info) {
		info = mpt_log_default();
	}
	
	if (!(conf = configNLS(nls->cfg))) {
		mpt_log(info, _func, MPT_LOG(Error), "%s",
		        MPT_tr("unable to get NLS client config"));
		return MPT_ERROR(BadOperation);
	}
	/* reevaluate solver config file */
	if ((sol = mpt_node_find(conf, "solconf", 1))
	    && !sol->children
	    && (val = mpt_node_data(sol, 0))) {
		MPT_STRUCT(parse) parse = MPT_PARSE_INIT;
		if (!(parse.src.arg = fopen(val, "r"))) {
			mpt_log(info, _func, MPT_LOG(Error), "%s: %s: %s",
			        MPT_tr("failed to open"), MPT_tr("solver config"), val);
			return MPT_ERROR(BadOperation);
		}
		parse.src.getc = (int (*)()) mpt_getchar_stdio;
		ret = mpt_parse_node(sol, &parse, "[ ] = !#");
		fclose(parse.src.arg);
		if (ret < 0) {
			mpt_log(info, _func, MPT_LOG(Error), "%s (%d): line = %d: %s",
			        MPT_tr("parse error"), parse.curr, (int) parse.src.line, val);
			return ret;
		}
		mpt_log(info, _func, MPT_CLIENT_LOG_STATUS, "%s: %s",
		        MPT_tr("loaded solver config file"), val);
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
	if ((mt = nls->out._graphic)) {
		MPT_INTERFACE(output) *out = 0;
		if (mt->_vptr->conv(mt, MPT_ENUM(TypeOutput), &out) >= 0
		    && out) {
			mpt_conf_graphic(out, conf->children, info);
		}
	}
	sol = mpt_node_find(conf, "solver", 1);
	val = sol ? mpt_node_data(sol, 0) : 0;
	
	if (!nls->sol
	    && !(nls->sol = (void *) mpt_solver_load(&nls->pr, MPT_SOLVER_ENUM(CapableNls), val, info))) {
		return MPT_ERROR(BadValue);
	}
	val = mpt_object_typename((void *) nls->sol);
	if (val) {
		mpt_log(info, 0, MPT_LOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	if ((ret = nls->uinit(nls->sol, nls->sd, info)) < 0) {
		return ret;
	}
	return 0;
}
static int stepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(iterator) *args)
{
	MPT_STRUCT(NLS) *nls = (void *) cl;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(solver_data) *dat;
	MPT_SOLVER(interface) *sol;
	MPT_STRUCT(node) *names;
	struct _outNLSdata ctx;
	struct rusage pre, post;
	const double *par;
	MPT_INTERFACE(logger) *info = 0;
	int res;
	
	(void) args;
	
	if (!(mt = nls->out._info)
	    || mt->_vptr->conv(mt, MPT_ENUM(TypeIterator), &info) < 0
	    || !info) {
		info = mpt_log_default();
	}
	
	if (!(sol = nls->sol) || !(dat = nls->sd)) {
		return -1;
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
	ctx.out = &nls->out;
	ctx.dat = dat;
	
	/* select output state */
	if (res < 0) {
		ctx.state = MPT_DATASTATE(Fail);
	} else if (res) {
		ctx.state = MPT_DATASTATE(Step);
	} else {
		ctx.state = MPT_DATASTATE(Fini) | MPT_DATASTATE(Step);
	}
	mpt_solver_status((void *) sol, info, outNLS, &ctx);
	
	/* no final output for state */
	if (res > 0) {
		return res;
	}
	
	if ((names = configNLS(nls->cfg))
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
			        desc, i+1, par[i]);
		}
	}
	mpt_solver_statistics((void *) sol, info, &nls->ru_usr, &nls->ru_sys);
	
	return res;
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
extern MPT_INTERFACE(client) *mpt_client_nls(MPT_INTERFACE(metatype) *out, int (*uinit)(MPT_SOLVER(interface) *, MPT_STRUCT(solver_data) *, MPT_INTERFACE(logger) *))
{
	static MPT_INTERFACE_VPTR(config) configNLS = {
		queryNLS, assignNLS, removeNLS
	};
	static MPT_INTERFACE_VPTR(client) clientNLS = {
		{ { deleteNLS, addrefNLS }, convNLS, cloneNLS },
		initNLS, stepNLS
	};
	const MPT_STRUCT(solver_output) def = MPT_SOLVER_OUTPUT_INIT;
	MPT_STRUCT(NLS) *nls;
	
	if (!uinit || !out) {
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
	
	nls->out = def;
	nls->out._data = out;
	
	nls->sol = 0;
	nls->uinit = uinit;
	memset(&nls->ru_usr, 0, sizeof(nls->ru_usr));
	memset(&nls->ru_sys, 0, sizeof(nls->ru_sys));
	
	return &nls->_cl;
}

