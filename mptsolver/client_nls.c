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
#include "parse.h"

#include "client.h"

#include "solver.h"

struct NLS {
	MPT_INTERFACE(client) cl;
	
	MPT_STRUCT(proxy) pr;
	
	MPT_SOLVER_STRUCT(data) *sd;
	char*cfg;
	
	MPT_SOLVER(NLS) *sol;
	int (*uinit)(MPT_SOLVER(NLS) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
	struct timeval  ru_usr,  /* user time in solver backend */
	                ru_sys;  /* system time in solver backend */
};
struct _outNLSdata
{
	MPT_INTERFACE(output) *out;
	MPT_SOLVER_STRUCT(data) *dat;
	int state;
};

static int outNLS(void *ptr, const MPT_STRUCT(value) *val)
{
	const struct _outNLSdata *ctx = ptr;
	int ret;
	
	/* copy parameters form solver output */
	if ((ret = mpt_data_nls(ctx->dat, val)) < 0) {
		return ret;
	}
	/* output of user data and residuals */
	if (ctx->out) {
		return mpt_output_nls(ctx->out, ctx->state, val, ctx->dat);
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
		if (mpt_config_set(0, base, "nls.conf", '.', 0) < 0) {
			return 0;
		}
		return mpt_config_node(base ? &p : 0);
	}
}

static void deleteNLS(MPT_INTERFACE(unrefable) *gen)
{
	struct NLS *nls = (void *) gen;
	
	mpt_proxy_fini(&nls->pr);
	
	if (nls->sd) {
		mpt_data_fini(nls->sd);
		free(nls->sd);
	}
	if (nls->cfg) {
		free(nls->cfg);
	}
	free(nls);
}
static MPT_INTERFACE(metatype) *queryNLS(const MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	const struct NLS *nls = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		return nls->pr._mt;
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
	p.flags &= ~MPT_ENUM(PathHasArray);
	
	if (!(conf = mpt_node_query(conf, &p, -1))) {
		return 0;
	}
	return conf->_meta;
}
static int assignNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val)
{
	static const char _func[] = "mpt::client<NLS>::assign";
	
	struct _outNLSdata ctx;
	struct NLS *nls = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(metatype) *mt;
	int ret;
	
	if (!(conf = configNLS(nls->cfg))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg) {
		MPT_INTERFACE(logger) *log;
		MPT_SOLVER(NLS) *sol;
		
		mt = nls->pr._mt;
		/* no prepare on output/solver assign */
		if ((ret = mpt_proxy_assign(&nls->pr, val)) < 0 || val) {
			if ((mt != nls->pr._mt)
			    && !(nls->sol = (void *) mpt_solver_load(&nls->pr, MPT_SOLVER_ENUM(CapableNls), 0))) {
				return MPT_ERROR(BadValue);
			}
			return ret;
		}
		log = nls->pr.logger;
		if (!(dat = nls->sd)) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
			                 MPT_tr("missing data descriptor"));
			return MPT_ERROR(BadOperation);
		}
		if (!(sol = nls->sol)) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
			                 MPT_tr("missing solver descriptor"));
			return MPT_ERROR(BadOperation);
		}
		mpt_solver_param((void *) sol, conf->children, 0, log);
		
		if ((ret = mpt_conf_history(nls->pr.output, conf->children)) < 0) {
			return ret;
		}
		/* set solver parameters from argument */
		if (log) {
			mpt_solver_info((void *) sol, log);
			mpt_log(log, 0, MPT_LOG(Message), "");
		}
		ctx.out = nls->pr.output;
		ctx.dat = dat;
		ctx.state = MPT_ENUM(DataStateInit);
		
		mpt_solver_status((void *) sol, log, outNLS, &ctx);
		
		return 0;
	}
	if (!porg->len) {
		MPT_INTERFACE(logger) *log = nls->pr.logger;
		ret = mpt_node_parse(conf, val, log);
		if (ret >= 0) {
			if (log) mpt_log(log, _func, MPT_CLIENT_LOGLEVEL, "%s: %s",
			                 MPT_tr("loaded NLS client config file"));
		}
		return ret;
	}
	if (!(conf = mpt_node_assign(&conf->children, porg))
	    || !(mt = conf->_meta)
	    || (ret = mt->_vptr->assign(mt, val)) < 0) {
		MPT_INTERFACE(logger) *log = nls->pr.logger;
		if (log) mpt_log(log, _func, MPT_LOG(Critical), "%s", MPT_tr("unable to assign client element"));
		return MPT_ERROR(BadOperation);
	}
	return ret;
}
static int removeNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	static const char _func[] = "mpt::client<NLS>::remove";
	
	struct NLS *nls = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		MPT_INTERFACE(metatype) *mt;
		MPT_INTERFACE(output) *out;
		
		/* close history output */
		if ((out = nls->pr.output) && mpt_conf_history(out, 0) < 0) {
			MPT_INTERFACE(logger) *log = nls->pr.logger;
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
			                 MPT_tr("unable to close history output"));
		}
		if (nls->sd) {
			mpt_data_clear(nls->sd);
		}
		if ((mt = nls->pr._mt)) {
			mt->_vptr->ref.unref((void *) mt);
			nls->pr._mt = 0;
			nls->pr.hash = 0;
			
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
	p.flags &= ~MPT_ENUM(PathHasArray);

	if (!(conf = mpt_node_query(conf->children, &p, -1))) {
		return MPT_ERROR(BadArgument);
	}
	mpt_node_clear(conf);
	return 1;
}

static int initNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	static const char _func[] = "mpt::client<NLS>::init";
	
	struct NLS *nls = (void *) cl;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_STRUCT(node) *conf, *sol;
	const char *val;
	int ret;
	
	(void) args;
	
	log = nls->pr.logger;
	
	if (!(conf = configNLS(nls->cfg))) {
		if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
		                 MPT_tr("unable to get NLS client config"));
		return MPT_ERROR(BadOperation);
	}
	/* reevaluate solver config file */
	if ((sol = mpt_node_find(conf, "solconf", 1))
	    && !sol->children
	    && (val = mpt_node_data(sol, 0))) {
		FILE *fd;
		if (!(fd = fopen(val, "r"))) {
			if (log) mpt_log(log, _func, MPT_LOG(Error), "%s: %s: %s",
			                 MPT_tr("failed to open"), MPT_tr("solver config"), val);
			return MPT_ERROR(BadOperation);
		}
		ret = mpt_node_read(sol, fd, "[ ] = !#", "ns", log);
		fclose(fd);
		if (ret < 0) {
			return ret;
		}
	}
	if (!(dat = nls->sd)) {
		if (!(dat = malloc(sizeof(*dat)))) {
			mpt_log(log, _func, MPT_LOG(Critical), "%s", MPT_tr("no memory for data"));
			return MPT_ERROR(BadOperation);
		}
		mpt_data_init(dat);
		nls->sd = dat;
	}
	/* clear existing data */
	else {
		mpt_data_clear(dat);
	}
	/* assign nls data */
	if ((ret = mpt_conf_nls(dat, conf->children, log)) < 0) {
		if (log) mpt_log(log, _func, MPT_LOG(Error), "%s",
		                 MPT_tr("solver preparation failed"));
		return ret;
	}
	/* rebind graphic associations */
	mpt_conf_graphic(nls->pr.output, conf->children);
	
	sol = mpt_node_find(conf, "solver", 1);
	val = sol ? mpt_node_data(sol, 0) : 0;
	nls->sol = (void *) mpt_solver_load(&nls->pr, MPT_SOLVER_ENUM(CapableNls), val);
	
	if (!nls->sol) {
		return MPT_ERROR(BadValue);
	}
	val = mpt_object_typename((void *) nls->sol);
	if (val && log) {
		mpt_log(log, 0, MPT_LOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	if ((ret = nls->uinit(nls->sol, nls->sd, log)) < 0) {
		return ret;
	}
	return 0;
}
static int stepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER(NLS) *sol;
	MPT_STRUCT(node) *names;
	struct _outNLSdata ctx;
	struct rusage pre, post;
	const double *par;
	int res;
	
	(void) args;
	
	log = nls->pr.logger;
	
	if (!(sol = (void *) nls->sol) || !(dat = nls->sd)) {
		return -1;
	}
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	res = sol->_vptr->solve(sol);
	
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_timeradd_sys(&nls->ru_sys, &pre, &post);
	mpt_timeradd_usr(&nls->ru_usr, &pre, &post);
	
	ctx.out = nls->pr.output;
	ctx.dat = dat;
	
	if (res < 0) {
		ctx.state = MPT_ENUM(DataStateFail);
		mpt_solver_status((void *) sol, log, outNLS, &ctx);
		return res;
	}
	if (res) {
		ctx.state = MPT_ENUM(DataStateStep);
		mpt_solver_status((void *) sol, log, outNLS, &ctx);
		return res;
	}
	ctx.state = MPT_ENUM(DataStateFini) | MPT_ENUM(DataStateStep);
	
	mpt_solver_status((void *) sol, log, outNLS, &ctx);
	
	if ((names = configNLS(nls->cfg))
	    && (names = mpt_node_find(names, "param", 1))) {
		names = names->children;
	}
	if ((par = mpt_data_param(dat))) {
		int i, npar;
		
		npar = nls->sd->npar;
		for (i = 0; i < npar; ++i) {
			const char *desc = MPT_tr("parameter");
			
			if (names) {
				const char *name = mpt_node_ident(names);
				names = names->next;
				if (name) {
					mpt_log(log, 0, MPT_LOG(Message), "%s %2d: %16g (%s)",
					        desc, i+1, par[i], name);
					continue;
				}
			}
			mpt_log(log, 0, MPT_LOG(Message), "%s %2d: %16g",
			        desc, i+1, par[i]);
		}
	}
	mpt_solver_statistics((void *) sol, log, &nls->ru_usr, &nls->ru_sys);
	
	return res;
}

static MPT_INTERFACE_VPTR(client) ctlNLS = {
	{ { deleteNLS }, queryNLS, assignNLS, removeNLS },
	initNLS, stepNLS
};

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
extern MPT_INTERFACE(client) *mpt_client_nls(int (*uinit)(MPT_SOLVER(NLS) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *), const char *base)
{
	struct NLS *nls;
	
	if (!uinit) {
		return 0;
	}
	if (!(nls = malloc(sizeof(*nls)))) {
		return 0;
	}
	/* query config base */
	if (!configNLS(base)
	    || (base && !(nls->cfg = strdup(base)))) {
		free(nls);
		return 0;
	}
	nls->cl._vptr = &ctlNLS;
	(void) memset(&nls->pr, 0, sizeof(nls->pr));
	nls->sd = 0;
	
	nls->sol = 0;
	nls->uinit = uinit;
	memset(&nls->ru_usr, 0, sizeof(nls->ru_usr));
	memset(&nls->ru_sys, 0, sizeof(nls->ru_sys));
	
	return &nls->cl;
}

