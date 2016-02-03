/*!
 * create client for solving nonlinear systems.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/resource.h>

#include "node.h"
#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "client.h"

#include "solver.h"

struct NLS {
	MPT_INTERFACE(client)    cl;
	MPT_STRUCT(proxy)        pr;
	MPT_SOLVER_STRUCT(data) *sd;
	MPT_SOLVER_INTERFACE    *sol;
	MPT_STRUCT(node)        *conf;
	int (*uinit)(MPT_SOLVER_STRUCT(nlsfcn) *, const MPT_SOLVER_STRUCT(data) *);
};

static void deleteNLS(MPT_INTERFACE(config) *gen)
{
	struct NLS *nls = (void *) gen;
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(object) *obj;
	
	if ((obj = (void*) nls->cl.out)) {
		obj->_vptr->unref(obj);
	}
	if ((m = nls->pr._ref)) {
		m->_vptr->unref(m);
	}
	if (nls->sd) {
		mpt_data_fini(nls->sd);
		free(nls->sd);
	}
	free(nls);
}

static int initNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	static const char from[] = "mpt::client<NLS>::init";
	struct NLS *nls = (void *) cl;
	MPT_STRUCT(node) *node;
	MPT_INTERFACE(logger) *log;
	const char *conf;
	int ret = 1;
	
	(void) args;
	
	log = mpt_object_logger((MPT_INTERFACE(object) *) nls->cl.out);
	
	if (!(node = mpt_node_find(nls->conf, "solver", 1))
	    || !(conf = mpt_node_data(node, 0))) {
		if (!nls->sol) {
			(void) mpt_log(log, from, MPT_FCNLOG(Error), "%s",
			               MPT_tr("missing solver description"));
			return MPT_ERROR(BadOperation);
		}
		ret = 0;
	}
	else {
		const char *a = mpt_solver_alias(conf);
		if (!(nls->sol = mpt_solver_load(&nls->pr, a ? a : conf, MPT_SOLVER_ENUM(CapableNls), log))) {
			return MPT_ERROR(BadType);
		}
	}
	conf = mpt_object_typename((void *) nls->sol);
	if (conf) {
		(void) mpt_log(log, 0, MPT_FCNLOG(Message), "%s: %s",
		               MPT_tr("solver"), conf);
	}
	if (!nls->sd) {
		if (!(nls->sd = malloc(sizeof(*nls->sd)))) {
			(void) mpt_log(log, from, MPT_FCNLOG(Critical), "%s",
			               MPT_tr("no memory for data"));
			return MPT_ERROR(BadOperation);
		}
		mpt_data_init(nls->sd);
	}
	mpt_conf_graphic(nls->cl.out, nls->conf->children);
	
	return ret;
}

static int prepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	static const char from[] = "mpt::client::prep/NLS";
	const struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_SOLVER_STRUCT(nlsfcn) *ufcn;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER_INTERFACE *gen;
	const MPT_INTERFACE_VPTR(Nls) *nctl;
	int32_t ret;
	
	log = mpt_object_logger((MPT_INTERFACE(object) *) nls->cl.out);
	
	if (!(gen = nls->sol)) {
		mpt_output_log(nls->cl.out, from, MPT_FCNLOG(Error), "%s",
		               MPT_tr("no solver assigned"));
		return MPT_ERROR(BadArgument);
	}
	if (!(dat = nls->sd)) {
		mpt_output_log(nls->cl.out, from, MPT_FCNLOG(Error), "%s",
		               MPT_tr("missing solver data"));
		return MPT_ERROR(BadArgument);
	}
	if ((ret = mpt_conf_history(nls->cl.out, nls->conf->children)) < 0) {
		return -1;
	}
	/* clear generated data */
	mpt_data_clear(dat);
	if ((ret = mpt_conf_nls(dat, nls->conf->children, log)) < 0) {
		mpt_output_log(nls->cl.out, from, MPT_FCNLOG(Error), "%s",
		              MPT_tr("solver preparation failed"));
		return ret;
	}
	/* call user init function */
	nctl = (void *) gen->_vptr;
	ufcn = nctl->functions(gen);
	if ((ret = nls->uinit(ufcn, dat)) < 0) {
		return ret;
	}
	if (dat->nval < dat->npar) {
		mpt_output_log(nls->cl.out, from, MPT_FCNLOG(Error), "%s: %i < %i",
		               MPT_tr("not enough parameters"), dat->nval, dat->npar);
		return MPT_ERROR(BadValue);
	}
	
	if (mpt_object_set((void *) gen, 0, dat->nval ? "ii" : "i", ret, dat->nval) <= 0) {
		mpt_output_log(nls->cl.out, from, MPT_FCNLOG(Error), "%s",
		               MPT_tr("unable to save problem dimensions to solver"));
		return MPT_ERROR(BadOperation);
	}
	mpt_solver_param((void *) gen, nls->conf->children, args, log);
	
	if (!log) {
		return 0;
	}
	mpt_solver_info(gen, log);
	mpt_log(log, 0, MPT_ENUM(LogMessage), "");
	mpt_solver_status(gen, log, 0, 0);
	
	return 0;
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
	const double *dat;
	int ret, i, ld, len;
	
	ret = 0;
	if (ctx->dat && (ret = mpt_data_nls(ctx->dat, val)) < 0) {
		return ret;
	}
	if (!ctx->out) {
		return ret;
	}
	ret = mpt_output_nls(ctx->out, ctx->state, val, 0);
	
	if (ret < 0 || !ctx->dat || !(ctx->state & MPT_ENUM(OutputStateFini))) {
		return ret;
	}
	/* output user data */
	dat = mpt_data_grid(ctx->dat);
	len = ctx->dat->nval;
	dat += len;
	ld = ((ctx->dat->val._buf->used / sizeof(*dat) - len)) / len;
	
	for (i = 0; i < ld; ++i) {
		if (mpt_bitmap_get(ctx->dat->mask, sizeof(ctx->dat->mask), i+1) > 0) {
			continue;
		}
		mpt_output_data(ctx->out, ctx->state, i+1, len, dat++, ld);
	}
	
	return 0;
}
static int stepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	const struct NLS *nls = (void *) cl;
	const MPT_INTERFACE_VPTR(Nls) *ctl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_SOLVER_INTERFACE *gen;
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(node) *names;
	struct _outNLSNdata ctx;
	struct rusage pre, post;
	const double *par;
	int res;
	
	(void) args;
	
	if (!(gen = (void *) nls->sol) || !(dat = nls->sd)) {
		return -1;
	}
	log = mpt_object_logger((MPT_INTERFACE(object) *) nls->cl.out);
	
	ctl = (void *) gen->_vptr;
	
	/* initialize current time structures */
	getrusage(RUSAGE_SELF, &pre);
	res = ctl->solve(gen);
	
	/* add solver runtime */
	getrusage(RUSAGE_SELF, &post);
	mpt_data_timeradd(dat, &pre, &post);
	
	ctx.out = nls->cl.out;
	ctx.dat = dat;
	
	if (res < 0) {
		ctx.state = MPT_ENUM(OutputStateFail);
		mpt_solver_status(gen, log, outNLS, &ctx);
		return res;
	}
	if (res) {
		ctx.state = MPT_ENUM(OutputStateStep);
		mpt_solver_status(gen, log, outNLS, &ctx);
		return res;
	}
	ctx.state = MPT_ENUM(OutputStateFini) | MPT_ENUM(OutputStateStep);
	
	mpt_solver_status(gen, log, outNLS, &ctx);
	
	if ((names = nls->conf)
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
					mpt_output_log(nls->cl.out, 0, MPT_FCNLOG(Message), "%s %2d: %16g (%s)",
					               desc, i+1, par[i], name);
					continue;
				}
			}
			mpt_output_log(nls->cl.out, 0, MPT_FCNLOG(Message), "%s %2d: %16g",
			               desc, i+1, par[i]);
		}
	}
	mpt_solver_statistics(gen, log, &dat->ru_usr, &dat->ru_sys);
	
	return res;
}

static void clearNLS(MPT_INTERFACE(client) *cl)
{
	const struct NLS *nls = (void *) cl;
	
	if (nls->conf) mpt_node_clear(nls->conf);
	
	/* close history output */
	if (nls->cl.out && mpt_conf_history(nls->cl.out, 0) < 0) {
		mpt_output_log(nls->cl.out, __func__, MPT_FCNLOG(Error), "%s",
		               MPT_tr("unable to close history output"));
	}
}

static MPT_INTERFACE_VPTR(client) ctlNLS = {
	{ deleteNLS, 0, 0, 0 },
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
extern MPT_INTERFACE(client) *mpt_client_nls(int (*uinit)(MPT_SOLVER_STRUCT(nlsfcn) *, const MPT_SOLVER_STRUCT(data) *), const char *base)
{
	struct NLS *nls;
	
	if (!(nls = malloc(sizeof(*nls)))) {
		return 0;
	}
	nls->cl._vptr = &ctlNLS;
	nls->cl.out = 0;
	
	(void) memset(&nls->pr, 0, sizeof(nls->pr));
	
	nls->sd = 0;
	nls->sol = 0;
	nls->conf = 0;
	
	(void) base;
	
	nls->uinit = uinit;
	
	return &nls->cl;
}

