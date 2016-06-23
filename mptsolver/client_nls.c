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

#include "values.h"
#include "output.h"

#include "client.h"

#include "solver.h"

struct NLS {
	MPT_INTERFACE(client) cl;
	
	MPT_SOLVER_STRUCT(clientdata) cd;
	
	MPT_SOLVER_STRUCT(data) *sd;
	char                    *cfg;
	
	MPT_SOLVER(NLS)         *sol;
	int (*uinit)(MPT_SOLVER(NLS) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
	struct timeval           ru_usr,   /* user time in solver backend */
	                         ru_sys;   /* system time in solver backend */
};

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

static void deleteNLS(MPT_INTERFACE(config) *gen)
{
	struct NLS *nls = (void *) gen;
	
	mpt_clientdata_fini(&nls->cd);
	
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
	
	if (!(conf = configNLS(nls->cfg))) {
		return 0;
	}
	if (!porg) {
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
	struct NLS *nls = (void *) gen;
	MPT_STRUCT(node) *conf;
	int ret;
	
	if (!(conf = configNLS(nls->cfg))) {
		return MPT_ERROR(BadOperation);
	}
	if (!val) {
		mpt_clientdata_assign(&nls->cd, val);
		ret = mpt_solver_assign(conf, porg, val, nls->cd.log);
	}
	else if (porg) {
		ret = mpt_solver_assign(conf, porg, val, nls->cd.log);
	}
	else {
		ret = mpt_clientdata_assign(&nls->cd, val);
	}
	nls->sol = mpt_proxy_cast(&nls->cd.pr, MPT_ENUM(TypeSolver));
	
	return ret;
}
static int removeNLS(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	const struct NLS *nls = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!(conf = configNLS(nls->cfg))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg) {
		return MPT_ERROR(BadArgument);
	}
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
	const char *name;
	int ret;
	
	(void) args;
	
	log = nls->cd.log;
	
	if (!(conf = configNLS(nls->cfg))) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to get NLS client config"));
		return MPT_ERROR(BadOperation);
	}
	if (!(sol = mpt_node_find(conf, "solver", 1))
	    || !(name = mpt_node_data(sol, 0))) {
		if (!nls->sol) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("missing solver description"));
			return MPT_ERROR(BadOperation);
		}
	}
	else {
		const char *a = mpt_solver_alias(name);
		int mode;
		
		if ((mode = mpt_solver_load(&nls->cd.pr, a ? a : name, log)) < 0) {
			return mode;
		}
		if (!(mode & MPT_SOLVER_ENUM(CapableNls))) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %i",
			                 MPT_tr("incompatible solver type"), mode);
			return MPT_ERROR(BadType);
		}
		nls->sol = mpt_proxy_cast(&nls->cd.pr, MPT_ENUM(TypeSolver));
	}
	name = mpt_object_typename((void *) nls->sol);
	if (name && log) {
		mpt_log(log, 0, MPT_FCNLOG(Message), "%s: %s", MPT_tr("solver"), name);
	}
	if (!(dat = nls->sd)) {
		if (!(dat = malloc(sizeof(*dat)))) {
			mpt_log(log, _func, MPT_FCNLOG(Critical), "%s", MPT_tr("no memory for data"));
			return MPT_ERROR(BadOperation);
		}
		mpt_data_init(dat);
		nls->sd = dat;
	}
	mpt_conf_graphic(nls->cd.out, conf->children);
	
	/* clear generated data */
	mpt_data_clear(dat);
	if ((ret = mpt_conf_nls(dat, conf->children, log)) < 0) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("solver preparation failed"));
		return ret;
	}
	if ((ret = nls->uinit(nls->sol, dat, log)) < 0) {
		return ret;
	}
	return ret;
}

struct _outNLSNdata
{
	MPT_INTERFACE(output) *out;
	MPT_SOLVER_STRUCT(data) *dat;
	int state;
};
static int outNLS(void *ptr, const MPT_STRUCT(value) *val)
{
	const struct _outNLSNdata *ctx = ptr;
	int ret = 0;
	
	if (ctx->out) {
		ret = mpt_output_nls(ctx->out, ctx->state, val, ctx->dat);
	}
	if ((ret = mpt_data_nls(ctx->dat, val)) < 0) {
		return ret;
	}
	/* residual data was taken from solver data */
	if (ret > 1) {
		ctx->dat->val._buf->used = ctx->dat->nval * sizeof(double);
		ctx->dat->nval = 0;
	}
	return ret;
}
static int prepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	static const char _func[] = "mpt::client::prep/NLS";
	
	struct _outNLSNdata ctx;
	const struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER(NLS) *sol;
	MPT_STRUCT(node) *conf;
	int ret;
	
	(void) args;
	
	log = nls->cd.log;
	
	if (!(conf = configNLS(nls->cfg))) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to get NLS client config"));
		return MPT_ERROR(BadOperation);
	}
	if (!(sol = nls->sol)) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("no solver assigned"));
		return MPT_ERROR(BadArgument);
	}
	if (!(dat = nls->sd)) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("missing solver data"));
		return MPT_ERROR(BadArgument);
	}
	if ((ret = mpt_conf_history(nls->cd.out, conf->children)) < 0) {
		return ret;
	}
	/* set solver parameters from argument */
	while (args) {
		MPT_STRUCT(property) pr;
		
		if ((ret = args->_vptr->conv(args, MPT_ENUM(TypeProperty) | MPT_ENUM(ValueConsume), &pr)) < 0) {
			break;
		}
		if (!ret) {
			args = 0;
			break;
		}
		if (!pr.name || !*pr.name) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("bad solver property name in arguments"));
			return MPT_ERROR(BadValue);
		}
		if ((ret = mpt_object_pset((void *) sol, pr.name, &pr.val, 0)) < 0) {
			if (ret == MPT_ERROR(BadArgument)) {
				if (log) mpt_log(log, _func, MPT_FCNLOG(Warning), "%s: %s",
				                 MPT_tr("bad property"), pr.name);
			} else if (pr.val.fmt) {
				if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s ('%s'): <%s>",
				                 MPT_tr("bad property value format"), pr.name, pr.val.fmt);
			} else if (pr.val.ptr) {
				if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s ('%s'): %s",
				                 MPT_tr("bad property value"), pr.name, pr.val.ptr);
			} else {
				if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s",
				                 MPT_tr("unable to reset property"), pr.name);
			}
		}
	}
	/* set initial values from argument */
	if (args && (ret = sol->_vptr->gen.obj.setProperty((void *) sol, 0, args)) < 0) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("invalid initial values in argument"));
		return ret;
	}
	if (log) {
		mpt_solver_info((void *) sol, log);
		mpt_log(log, 0, MPT_ENUM(LogMessage), "");
	}
	ctx.out = nls->cd.out;
	ctx.dat = dat;
	ctx.state = MPT_ENUM(OutputStateInit);
	
	mpt_solver_status((void *) sol, log, outNLS, &ctx);
	
	return 0;
}
static int stepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER(NLS) *sol;
	MPT_STRUCT(node) *names;
	struct _outNLSNdata ctx;
	struct rusage pre, post;
	const double *par;
	int res;
	
	(void) args;
	
	log = nls->cd.log;
	
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
	
	ctx.out = nls->cd.out;
	ctx.dat = dat;
	
	if (res < 0) {
		ctx.state = MPT_ENUM(OutputStateFail);
		mpt_solver_status((void *) sol, log, outNLS, &ctx);
		return res;
	}
	if (res) {
		ctx.state = MPT_ENUM(OutputStateStep);
		mpt_solver_status((void *) sol, log, outNLS, &ctx);
		return res;
	}
	ctx.state = MPT_ENUM(OutputStateFini) | MPT_ENUM(OutputStateStep);
	
	mpt_solver_status((void *) sol, log, outNLS, &ctx);
	
	if ((names = configNLS(nls->cfg))
	    && (names = names->children)
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
					mpt_log(log, 0, MPT_FCNLOG(Message), "%s %2d: %16g (%s)",
					        desc, i+1, par[i], name);
					continue;
				}
			}
			mpt_log(log, 0, MPT_FCNLOG(Message), "%s %2d: %16g",
			        desc, i+1, par[i]);
		}
	}
	mpt_solver_statistics((void *) sol, log, &nls->ru_usr, &nls->ru_sys);
	
	return res;
}

static void clearNLS(MPT_INTERFACE(client) *cl)
{
	const struct NLS *nls = (void *) cl;
	MPT_STRUCT(node) *conf;
	
	if (!nls->cfg) {
		conf = mpt_config_node(0);
	}
	else {
		MPT_STRUCT(path) p = MPT_PATH_INIT;
		mpt_path_set(&p, nls->cfg, -1);
		conf = mpt_config_node(&p);
	}
	if (conf) {
		mpt_node_clear(conf);
	}
	/* close history output */
	if (nls->cd.out && mpt_conf_history(nls->cd.out, 0) < 0) {
		MPT_INTERFACE(logger) *log = nls->cd.log;
		if (log) mpt_log(log, __func__, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("unable to close history output"));
	}
}

static MPT_INTERFACE_VPTR(client) ctlNLS = {
	{ deleteNLS, queryNLS, assignNLS, removeNLS },
	initNLS, prepNLS, stepNLS, clearNLS
};

/*!
 * \ingroup mptSolver
 * \brief NLS client creation
 * 
 * Create client for solving Nonlinear Systems.
 * 
 * \param uinit user initialization function
 * 
 * \return NLS client
 */
extern MPT_INTERFACE(client) *mpt_client_nls(int (*uinit)(MPT_SOLVER(NLS) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *), const char *base)
{
	struct NLS *nls;
	
	if (!(nls = malloc(sizeof(*nls)))) {
		return 0;
	}
	nls->cl._vptr = &ctlNLS;
	
	(void) memset(&nls->cd, 0, sizeof(nls->cd));
	
	nls->sd = 0;
	/* query config base */
	if (!configNLS(base)
	    || !(base && (nls->cfg = strdup(base)))) {
		free(nls);
		return 0;
	}
	
	nls->sol = 0;
	nls->uinit = uinit;
	memset(&nls->ru_usr, 0, sizeof(nls->ru_usr));
	memset(&nls->ru_sys, 0, sizeof(nls->ru_sys));
	
	return &nls->cl;
}

