/*!
 * create client for IVP problem types
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

struct IVP {
	MPT_INTERFACE(client) cl;
	
	MPT_STRUCT(proxy) pr;
	
	MPT_INTERFACE(metatype) *steps;
	MPT_SOLVER_STRUCT(data) *sd;
	char *cfg;
	
	MPT_SOLVER(IVP) *sol;
	int (*uinit)(MPT_SOLVER(IVP) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *);
	struct timeval ru_usr,   /* user time in solver backend */
	               ru_sys;   /* system time in solver backend */
	double t;
	int pdim;
};

/* output for PDE solvers */
struct _clientPdeOut
{
	MPT_INTERFACE(output) *out;
	MPT_SOLVER_STRUCT(data) *dat;
	int state;
};
static int outPDE(void *ptr, const MPT_STRUCT(value) *val)
{
	struct _clientPdeOut *ctx = ptr;
	return mpt_output_pde(ctx->out, ctx->state, val, ctx->dat);
}

static MPT_STRUCT(node) *configIVP(const char *base)
{
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_STRUCT(node) *conf;
	
	mpt_path_set(&p, base, -1);
	if ((conf = mpt_config_node(base ? &p : 0))) {
		return conf;
	}
	if (mpt_config_set(0, base, "", '.', 0) < 0) {
		return 0;
	}
	return mpt_config_node(base ? &p : 0);
}
/* destruktor */
static void deleteIVP(MPT_INTERFACE(config) *gen)
{
	struct IVP *ivp = (void *) gen;
	MPT_INTERFACE(metatype) *m;
	
	mpt_proxy_fini(&ivp->pr);
	
	if ((m = ivp->steps)) {
		m->_vptr->unref(m);
	}
	if (ivp->sd) {
		mpt_data_fini(ivp->sd);
		free(ivp->sd);
	}
	if (ivp->cfg) {
		free(ivp->cfg);
	}
	free(ivp);
}
static MPT_INTERFACE(metatype) *queryIVP(const MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	const struct IVP *ivp = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!(conf = configIVP(ivp->cfg))) {
		return 0;
	}
	if (!porg) {
		return ivp->pr._mt;
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
static int assignIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg, const MPT_STRUCT(value) *val)
{
	static const char _func[] = "mpt::client<IVP>::assign";
	struct IVP *ivp = (void *) gen;
	MPT_INTERFACE(metatype) *mt;
	MPT_STRUCT(node) *conf;
	int ret;
	
	if (!(conf = configIVP(ivp->cfg))) {
		return MPT_ERROR(BadOperation);
	}
	if (!porg) {
		MPT_SOLVER_STRUCT(data) *dat;
		MPT_INTERFACE(logger) *log;
		MPT_SOLVER(IVP) *sol;
		
		mt = ivp->pr._mt;
		/* no prepare on output/solver assign */
		if ((ret = mpt_proxy_assign(&ivp->pr, val)) < 0 || val) {
			if ((mt != ivp->pr._mt)
			    && !(ivp->sol = (void *) mpt_solver_load(&ivp->pr, MPT_SOLVER_ENUM(CapableIvp), 0))) {
				return MPT_ERROR(BadValue);
			}
			return ret;
		}
		log = ivp->pr.logger;
		if (!(dat = ivp->sd)) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("missing data descriptor"));
			return MPT_ERROR(BadOperation);
		}
		if (!(sol = ivp->sol)) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("missing solver descriptor"));
			return MPT_ERROR(BadOperation);
		}
		/* setup history */
		if ((ret = mpt_conf_history(ivp->pr.output, conf->children)) < 0) {
			return ret;
		}
		/* set solver parameters */
		mpt_solver_param((void *) sol, conf->children, 0, log);
		
		/* prepare solver */
		if ((ret = sol->_vptr->step(sol, 0)) < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("solver backend prepare failed"));
			return ret;
		}
		if (log) {
			mpt_solver_info((void *) sol, log);
			mpt_log(log, 0, MPT_ENUM(LogMessage), "");
		}
		if (!ivp->pdim || !ivp->pr.output) {
			mpt_solver_status((void *) sol, log, 0, 0);
		}
		else {
			struct _clientPdeOut ctx;
			ctx.out = ivp->pr.output;
			ctx.dat = ivp->sd;
			ctx.state = MPT_ENUM(DataStateInit);
			mpt_solver_status((void *) sol, log, outPDE, &ctx);
		}
		if (log) mpt_log(log, _func, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("IVP client preparation finished"));
		
		return 0;
	}
	if (!porg->len) {
		MPT_INTERFACE(logger) *log = ivp->pr.logger;
		ret = mpt_node_parse(conf, val, log);
		if (ret >= 0) {
			if (log) mpt_log(log, _func, MPT_CLIENT_LOGLEVEL, "%s",
			                 MPT_tr("loaded IVP client config file"));
		}
		return ret;
	}
	if (!(conf = mpt_node_assign(&conf, porg))
	    || !(mt = conf->_meta)
	    || (ret = mt->_vptr->assign(mt, val)) < 0) {
		MPT_INTERFACE(logger) *log = ivp->pr.logger;
		if (log) mpt_log(log, _func, MPT_FCNLOG(Critical), "%s", MPT_tr("unable to assign client element"));
		return MPT_ERROR(BadOperation);
	}
	return ret;
}
static int removeIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	static const char _func[] = "mpt::client<IVP>::remove";
	
	struct IVP *ivp = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!porg) {
		MPT_INTERFACE(metatype) *mt;
		MPT_INTERFACE(output) *out;
		
		/* close history output */
		if ((out = ivp->pr.output) && mpt_conf_history(out, 0) < 0) {
			MPT_INTERFACE(logger) *log = ivp->pr.logger;
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("unable to close history output"));
		}
		if (ivp->sd) {
			mpt_data_clear(ivp->sd);
		}
		if ((mt = ivp->pr._mt)) {
			mt->_vptr->unref(mt);
			ivp->pr._mt = 0;
			ivp->pr.hash = 0;
			
			ivp->sol = 0;
			return 1;
		}
		return 0;
	}
	if (!(conf = configIVP(ivp->cfg))) {
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
/* initialisation */
static int initIVP(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	static const char _func[] = "mpt::client<IVP>::init";
	
	struct IVP *ivp = (void *) cl;
	MPT_STRUCT(node) *conf, *curr;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(metatype) *m;
	MPT_INTERFACE(logger) *log;
	const char *val;
	int ret;
	
	(void) args;
	
	log = ivp->pr.logger;
	
	if (!(conf = configIVP(ivp->cfg))) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s",
		                 MPT_tr("failed to query"), MPT_tr("client configuration"));
		return MPT_ERROR(BadOperation);
	}
	/* reevaluate solver config file */
	if ((curr = mpt_node_find(conf, "solconf", 1))
	    && !curr->children
	    && (val = mpt_node_data(curr, 0))) {
		FILE *fd;
		if (!(fd = fopen(val, "r"))) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s: %s",
			                 MPT_tr("failed to open"), MPT_tr("solver config"), val);
			return MPT_ERROR(BadOperation);
		}
		ret = mpt_node_read(curr, fd, "[ ] = !#", "ns", log);
		fclose(fd);
		if (ret < 0) {
			return ret;
		}
		if (log) mpt_log(log, _func, MPT_CLIENT_LOGLEVEL, "%s: %s",
		                 MPT_tr("loaded solver config file"), val);
	}
	/* create time step source */
	if (!(curr = mpt_node_find(conf, "times", 1))) {
		m = ivp->steps;
	}
	else {
		val = mpt_node_data(curr, 0);
		
		if (!val || !(m = mpt_iterator_create(val))) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s",
			                 MPT_tr("failed to query"), MPT_tr("client configuration"));
			return MPT_ERROR(BadValue);
		}
		if (ivp->steps) {
			ivp->steps->_vptr->unref(ivp->steps);
		}
		ivp->steps = m;
	}
	ivp->t = 0;
	if (m && (ret = m->_vptr->conv(m, 'd' | MPT_ENUM(ValueConsume), &ivp->t)) <= 0) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s",
		                 MPT_tr("failed to query"), MPT_tr("initial time value"));
		return MPT_ERROR(BadValue);
	}
	/* clear existing solver data */
	if ((dat = ivp->sd)) {
		mpt_data_clear(dat);
	}
	/* create/initialize new solver data */
	else if ((dat = malloc(sizeof(*dat)))) {
		mpt_data_init(dat);
		ivp->sd = dat;
	}
	else {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s: %s",
		                 MPT_tr("failed to create"), MPT_tr("solver data"));
		return MPT_ERROR(BadOperation);
	}
	/* get PDE grid data */
	if ((curr = mpt_node_next(conf->children, "grid"))) {
		if ((ret = mpt_conf_grid(&dat->val, curr)) < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Warning), "%s: %s",
			                 "grid", MPT_tr("invalid parameter format"));
		}
		dat->nval = ret;
	}
	/* setup ODE mode */
	else {
		/* save initial time value */
		if (!mpt_array_append(&dat->val, sizeof(ivp->t), &ivp->t)) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("unable to save initial time in history"));
			return MPT_ERROR(BadOperation);
		}
		/* add profile values */
		if ((curr = mpt_node_next(conf->children, "profile"))
		    && (ret = mpt_conf_param(&dat->val, curr, 1)) < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("failed to reserve initial data"));
			return ret;
		}
	}
	/* get user parameter */
	if ((curr = conf->children)
	    && (curr = mpt_node_next(curr, "param"))) {
		if ((ret = mpt_conf_param(&dat->param, curr, 0)) < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Warning), "%s: %s",
			                 "param", MPT_tr("invalid parameter format"));
		}
		dat->npar = ret;
	}
	mpt_conf_graphic(ivp->pr.output, conf);
	
	/* load new solver or evaluate existing */
	curr = mpt_node_find(conf, "solver", 1);
	val = curr ? mpt_node_data(curr, 0) : 0;
	ivp->sol = (void *) mpt_solver_load(&ivp->pr, MPT_SOLVER_ENUM(CapableIvp), val);
	
	if (!ivp->sol) {
		return MPT_ERROR(BadValue);
	}
	val = mpt_object_typename((void *) ivp->sol);
	if (val && log) {
		mpt_log(log, 0, MPT_FCNLOG(Message), "%s: %s", MPT_tr("solver"), val);
	}
	
	if ((ret = ivp->uinit(ivp->sol, dat, log)) < 0) {
		return ret;
	}
	/* setup PDE time data */
	if ((ivp->pdim = dat->nval)) {
		int err;
		if ((err = mpt_object_set((void *) ivp->sol, 0, "d", ivp->t)) < 0) {
			if (log) mpt_log(log, 0, MPT_FCNLOG(Error), "%s: %s", MPT_tr("solver"), MPT_tr("failed to set initial time"));
			return err;
		}
	}
	/* save ODE/PDE segment size */
	else {
		dat->nval = dat->val._buf->used / sizeof(double);
	}
	/* process profile data */
	if (ret && ivp->pdim) {
		MPT_STRUCT(node) *pcfg;
		const double *grid;
		double *pdata;
		
		if (!(pdata = ivp->sol->_vptr->initstate(ivp->sol))) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("failed to get initial state"));
			return MPT_ERROR(BadOperation);
		}
		pcfg = mpt_node_next(conf->children, "profile");
		grid = mpt_data_grid(dat);
		
		if ((ret = mpt_conf_profiles(dat->nval, pdata, ret, pcfg, grid, log)) < 0) {
			return ret;
		}
	}
	if (ret < 0) {
		return ret;
	}
	if (log) mpt_log(log, _func, MPT_CLIENT_LOGLEVEL, "%s", MPT_tr("IVP client preparation finished"));
	
	return ret;
}
/* step operation on solver */
static int stepIVP(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *arg)
{
	static const char _func[] = "mpt::client<IVP>::step";
	
	struct IVP *ivp = (void *) cl;
	MPT_INTERFACE(metatype) *src;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER(IVP) *sol;
	struct _clientPdeOut ctx;
	struct rusage pre, post;
	double end;
	int ret;
	
	log = ivp->pr.logger;
	
	if (!(sol = ivp->sol) || !(ctx.dat = ivp->sd)) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("client not prepared for step operation"));
		return MPT_ERROR(BadOperation);
	}
	if (!(src = arg) && !(src = ivp->steps)) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("no time step source"));
		return MPT_ERROR(BadArgument);
	}
	/* execute ODE steps */
	if (!ivp->pdim) {
		const double *val;
		int i, ld, len;
		
		/* current time data */
		getrusage(RUSAGE_SELF, &pre);
		/* execute possible steps */
		ret = mpt_steps_ode(ivp->sol, src, ivp->sd, log);
		/* add time difference */
		getrusage(RUSAGE_SELF, &post);
		mpt_timeradd_sys(&ivp->ru_sys, &pre, &post);
		mpt_timeradd_usr(&ivp->ru_usr, &pre, &post);
		
		ctx.state = MPT_ENUM(DataStateStep);
		if (!ret) {
			ctx.state |= MPT_ENUM(DataStateFini);
		}
		else if (ret < 0) {
			ctx.state |= MPT_ENUM(DataStateFail);
		}
		
		ld  = ctx.dat->nval;
		val = mpt_data_grid(ctx.dat);
		len = ctx.dat->val._buf->used / sizeof(*val) / ld;
		
		mpt_output_history(ivp->pr.output, len, val, 1, 0, ld - 1);
		
		for (i = 0; i < ld; i++) {
			if (mpt_bitmap_get(ctx.dat->mask, sizeof(ctx.dat->mask), i) > 0) {
				continue;
			}
			mpt_output_data(ivp->pr.output, ctx.state, i, len, val+i, ld);
		}
		if (!ret && log) {
			mpt_solver_statistics((void *) sol, log, &ivp->ru_usr, &ivp->ru_sys);
		}
		return ret;
	}
	/* get current target from time source */
	if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		                 MPT_tr("bad value on time argument"));
		return ret;
	}
	if (!ret) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Warning), "%s",
		                 MPT_tr("no further values in time source"));
		return 0;
	}
	/* time soure date too low */
	while (end <= ivp->t) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Info), "%s (%g <= %g)",
		                 MPT_tr("skip time value argument"), end, ivp->t);
		
		if ((ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &end)) < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("bad value on time argument"));
			return ret;
		}
		if (!ret) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Info), "%s",
			                 MPT_tr("no further data in time argument"));
			return 0;
		}
		if (!(ret & MPT_ENUM(ValueConsume))) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("time source is not signaling advancement"));
			return MPT_ERROR(BadOperation);
		}
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("bad value on time argument"));
			return ret;
		}
		if (!ret) {
			if (log) mpt_log(log, _func, MPT_FCNLOG(Info), "%s",
			                 MPT_tr("no further data in time argument"));
			return 0;
		}
	}
	
	while (1) {
		ctx.state = MPT_ENUM(DataStateStep);
		ret = 1;
		
		getrusage(RUSAGE_SELF, &pre);
		
		ivp->t = end;
		if ((ret = sol->_vptr->step(sol, &ivp->t)) < 0) {
			ctx.state |= MPT_ENUM(DataStateFail);
			if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s (t = %g)",
			                 MPT_tr("solver step failed"), end);
			break;
		}
		getrusage(RUSAGE_SELF, &post);
		mpt_timeradd_sys(&ivp->ru_sys, &pre, &post);
		mpt_timeradd_usr(&ivp->ru_usr, &pre, &post);
		
		if (ivp->t < end) {
			mpt_solver_status((void *) sol, log, outPDE, &ctx);
			continue;
		}
		
		ret = 1;
		while (1) {
			int curr;
			if ((curr = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &end)) < 0) {
				if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
				                 MPT_tr("bad argument on time source"));
				ret = curr;
				break;
			}
			if (!curr) {
				ret = 0;
				ctx.state |= MPT_ENUM(DataStateFini);
				break;
			}
			if ((curr = src->_vptr->conv(src, 'd', &end)) < 0) {
				if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
				                 MPT_tr("bad argument on time source"));
				ret = curr;
				break;
			}
			if (!curr) {
				ret = 0;
				ctx.state |= MPT_ENUM(DataStateFini);
				break;
			}
			if (end > ivp->t) {
				break;
			}
			if (log) mpt_log(log, _func, MPT_FCNLOG(Info), "%s (%g < %g)",
			                 MPT_tr("skip time value argument"), end, ivp->t);
		}
		if ((ctx.out = ivp->pr.output)) {
			mpt_solver_status((void *) sol, log, outPDE, &ctx);
		}
		if (!ret && log) {
			mpt_solver_statistics((void *) sol, log, &ivp->ru_usr, &ivp->ru_sys);
		}
		if (!arg || ret < 1) {
			break;
		}
	}
	return ret;
}

static const MPT_INTERFACE_VPTR(client) clientIVP = {
	{ deleteIVP, queryIVP, assignIVP, removeIVP },
	initIVP, stepIVP
};

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
extern MPT_INTERFACE(client) *mpt_client_ivp(int (*uinit)(MPT_SOLVER(IVP) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(logger) *), const char *base)
{
	struct IVP *ivp;
	
	if (!uinit) {
		return 0;
	}
	if (!(ivp = malloc(sizeof(*ivp)))) {
		return 0;
	}
	/* query config base */
	if (!configIVP(base)
	    || !(base && (ivp->cfg = strdup(base)))) {
		free(ivp);
		return 0;
	}
	ivp->cl._vptr = &clientIVP;
	memset(&ivp->pr, 0, sizeof(ivp->pr));
	ivp->sd = 0;
	ivp->steps = 0;
	
	ivp->sol = 0;
	ivp->uinit = uinit;
	memset(&ivp->ru_usr, 0, sizeof(ivp->ru_usr));
	memset(&ivp->ru_sys, 0, sizeof(ivp->ru_sys));
	ivp->t = 0.0;
	ivp->pdim = 0;
	
	return &ivp->cl;
}

