/*!
 * create client for IVP problem types
 */

#define _POSIX_C_SOURCE 200809L /* need for strdup() */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/uio.h>

#include "node.h"
#include "message.h"
#include "array.h"
#include "parse.h"

#include "values.h"
#include "output.h"

#include "client.h"

#include "solver.h"

struct IVP {
	MPT_INTERFACE(client)    cl;
	MPT_STRUCT(proxy)        pr;
	MPT_SOLVER_STRUCT(data) *sd;
	MPT_SOLVER(IVP)         *sol;
	MPT_INTERFACE(metatype) *src;
	char                    *cfg;
	double                   t;
	int (*uinit)(void *, const MPT_SOLVER_STRUCT(data) *);
};

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
	MPT_INTERFACE(object) *obj;
	
	if ((obj = (void*) ivp->cl.out)) {
		obj->_vptr->unref(obj);
	}
	if ((m = ivp->pr._ref)) {
		m->_vptr->unref(m);
	}
	if (ivp->sd) {
		mpt_data_fini(ivp->sd);
		free(ivp->sd);
	}
	if ((m = ivp->src)) {
		m->_vptr->unref(m);
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
	const struct IVP *ivp = (void *) gen;
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(node) *conf;
	
	if (!(conf = configIVP(ivp->cfg))) {
		return MPT_ERROR(BadOperation);
	}
	log = mpt_object_logger((void *) ivp->cl.out);
	return mpt_solver_assign(conf, porg, val, log);
}
static int removeIVP(MPT_INTERFACE(config) *gen, const MPT_STRUCT(path) *porg)
{
	const struct IVP *ivp = (void *) gen;
	MPT_STRUCT(node) *conf;
	MPT_STRUCT(path) p;
	
	if (!(conf = configIVP(ivp->cfg))) {
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
/* initialisation */
static int initIVP(struct IVP *ivp, int type)
{
	static const char _func[] = "mpt::client<IVP>::init";
	
	MPT_STRUCT(node) *conf, *curr;
	MPT_INTERFACE(metatype) *m;
	const char *val;
	int ret = 1;
	
	if (!(conf = configIVP(ivp->cfg))) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s: %s",
		               MPT_tr("failed to query"), MPT_tr("client configuration"));
		return MPT_ERROR(BadOperation);
	}
	if (!(curr = mpt_node_find(conf, "times", 1))) {
		m = ivp->src;
	}
	else {
		val = mpt_node_data(curr, 0);
		
		if (!val || !(m = mpt_iterator_create(val))) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s: %s",
			               MPT_tr("failed to query"), MPT_tr("client configuration"));
			return MPT_ERROR(BadValue);
		}
		if (ivp->src) {
			ivp->src->_vptr->unref(ivp->src);
		}
		ivp->src = m;
	}
	ivp->t = 0;
	
	if (m && (ret = m->_vptr->conv(m, 'd' | MPT_ENUM(ValueConsume), &ivp->t)) <= 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s: %s",
		               MPT_tr("failed to query"), MPT_tr("initial time value"));
		return MPT_ERROR(BadValue);
	}
	if (!ivp->sd) {
		MPT_SOLVER_STRUCT(data) *dat;
		
		if (!(dat = malloc(sizeof(*dat)))) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s: %s",
			               MPT_tr("failed to create"), MPT_tr("solver data"));
			return MPT_ERROR(BadOperation);
		}
		mpt_data_init(dat);
		ivp->sd = dat;
	}
	
	if ((curr = mpt_node_find(conf, "solver", 1))) {
		val = mpt_node_data(curr, 0);
	} else {
		val = 0;
	}
	if (!val) {
		if (!ivp->sol) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
			                 MPT_tr("missing solver description"));
			return MPT_ERROR(BadOperation);
		}
		ret = 0;
	}
	else {
		MPT_INTERFACE(logger) *log;
		const char *res;
		
		if (!(res = mpt_solver_alias(val))) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s: %s",
			               MPT_tr("bad solver alias"), val);
			return MPT_ERROR(BadValue);
		}
		log = mpt_object_logger((void *) ivp->cl.out);
		if (!(ivp->sol = (void *) mpt_solver_load(&ivp->pr, res, type, log))) {
			return MPT_ERROR(BadType);
		}
		ret = 1;
	}
	mpt_conf_graphic(ivp->cl.out, conf);
	
	return ret;
}
static int initDiff(struct IVP *ivp, MPT_INTERFACE(metatype) *args, int type)
{
	static const char _func[] = "mpt::client<IVP>::vinit";
	static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
	
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER(IVP) *sol;
	void *fcn;
	MPT_STRUCT(node) *conf;
	struct {
		double t;
		struct iovec val;
	} tmp;
	MPT_STRUCT(value) val;
	double *ptr;
	int32_t neqs;
	int ret;
	
	(void) args;
	if ((ret = initIVP(ivp, type)) < 0) {
		return ret;
	}
	log = mpt_object_logger((void *) ivp->cl.out);
	dat = ivp->sd;
	
	if ((conf = configIVP(ivp->cfg))) {
		conf = conf->children;
	}
	/* set ODE parameter */
	mpt_data_clear(dat);
	if ((ret = mpt_conf_ode(dat, ivp->t, conf, log)) < 0) {
		return ret;
	}
	/* user configuration */
	sol = ivp->sol;
	fcn = sol->_vptr->functions(sol, type);
	if ((ret = ivp->uinit(fcn, dat)) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("user init function encountered error"));
		return ret;
	}
	if (!ret) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("user init function must return equotation count"));
		return MPT_ERROR(BadValue);
	}
	/* get IVP state data */
	if (!(ptr = mpt_data_grid(dat))) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("missing initial state data"));
		return MPT_ERROR(BadArgument);
	}
	/* set equotation count */
	neqs = ret;
	val.fmt = "i";
	val.ptr = &neqs;
	if ((neqs = mpt_object_pset((void *) ivp->sol, "", &val, 0)) <= 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("unable to save problem parameter to solver"));
		return neqs;
	}
	neqs = dat->nval - 1;
	dat->nval = ret + 1;
	if ((neqs = dat->nval - 1) < ret) {
		mpt_data_grid(dat);
	}
	else if (neqs > ret) {
		dat->val._buf->used = (ret + 1) * sizeof(double);
	}
	/* initial values for ODE/DAE */
	tmp.t = ivp->t;
	tmp.val.iov_base = ((double *) ptr) + 1;
	tmp.val.iov_len  = ret * sizeof(*ptr);
	
	val.fmt = fmt;
	val.ptr = &tmp;
	if ((neqs = mpt_object_pset((void *) ivp->sol, 0, &val, 0)) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("failed to set initial values"));
		return neqs;
	}
	return ret;
}
static int initDAE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	struct IVP *ivp = (void *) cl;
	return initDiff(ivp, args, MPT_SOLVER_ENUM(DAE));
}
static int initODE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	struct IVP *ivp = (void *) cl;
	return initDiff(ivp, args, MPT_SOLVER_ENUM(ODE));
}
static int initPDE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *args)
{
	static const char _func[] = "mpt::client<PDE>::init";
	struct IVP *ivp = (void *) cl;
	MPT_SOLVER_STRUCT(pdefcn) *pde;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(node) *conf;
	MPT_SOLVER(IVP) *sol;
	double *grid, *profile;
	int ret, len;
	
	(void) args;
	if ((ret = initIVP(ivp, MPT_SOLVER_ENUM(PDE))) < 0) {
		return ret;
	}
	sol = ivp->sol;
	log = mpt_object_logger((void *) ivp->cl.out);
	dat = ivp->sd;
	
	if ((conf = configIVP(ivp->cfg))) {
		conf = conf->children;
	}
	/* set PDE parameter */
	mpt_data_clear(dat);
	if ((len = mpt_conf_pde(dat, conf, log)) < 0) {
		return len;
	}
	grid = mpt_data_grid(dat);
	/* trigger PDE mode in solver */
	if (mpt_object_set((void *) sol, "", "ii", 1, len) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s", MPT_tr("failed to set pde dimension"));
		return MPT_ERROR(BadOperation);
	}
	/* user configuration */
	pde = sol->_vptr->functions(sol, MPT_SOLVER_ENUM(PDE));
	if (pde) pde->grid = grid;
	
	if ((ret = ivp->uinit(pde, dat)) < 0) {
		return ret;
	}
	if (mpt_object_set((void *) ivp->sol, "", "ii", ret, len) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s", MPT_tr("failed to set pde equotation count"));
		return MPT_ERROR(BadOperation);
	}
	if (mpt_object_set((void *) ivp->sol, 0, "d", ivp->t) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s", MPT_tr("failed to set initial time"));
		return MPT_ERROR(BadOperation);
	}
	if (!(profile = sol->_vptr->initstate(sol))) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s", MPT_tr("failed to get initial state"));
		return MPT_ERROR(BadOperation);
	}
	if ((len = mpt_conf_profiles(dat->nval, profile, ret, mpt_node_next(conf, "profile"), grid, log)) < 0) {
		return len;
	}
	return ret;
}
/* combined IVP preparation part */
static int prepIVP(const struct IVP *ivp, MPT_INTERFACE(metatype) *arg)
{
	static const char _func[] = "mpt::client<IVP>::prep";
	
	MPT_INTERFACE(logger) *log;
	MPT_STRUCT(node) *conf;
	MPT_SOLVER(IVP) *sol;
	int ret;
	
	log = mpt_object_logger((MPT_INTERFACE(object) *) ivp->cl.out);
	
	if (!(sol = ivp->sol)) {
		mpt_log(log, _func, MPT_FCNLOG(Error), "%s",
		        MPT_tr("failed to set initial values"));
		return MPT_ERROR(BadArgument);
	}
	if ((conf = configIVP(ivp->cfg))) {
		conf = conf->children;
	}
	if ((ret = mpt_conf_history(ivp->cl.out, conf)) < 0) {
		return ret;
	}
	mpt_solver_param((void *) sol, conf, arg, log);
	
	if ((ret = sol->_vptr->step(sol, 0)) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("preparing solver backend failed"));
		return ret;
	}
	if (log) {
		const char *name;
		
		if ((name = mpt_object_typename((void *) sol))) {
			mpt_log(log, 0, MPT_ENUM(LogMessage), "%s: %s", MPT_tr("solver"), name);
		}
		mpt_solver_info((void *) sol, log);
		mpt_log(log, 0, MPT_ENUM(LogMessage), "");
	}
	return ret;
}
static int prepODE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *arg)
{
	const struct IVP *ivp = (void *) cl;
	MPT_INTERFACE(output) *out;
	MPT_INTERFACE(logger) *log;
	int ret;
	
	if ((ret = prepIVP(ivp, arg)) < 0) {
		return ret;
	}
	if (!(out = ivp->cl.out)) {
		return ret;
	}
	log = mpt_object_logger((void *) out);
	mpt_solver_status((void *) ivp->sol, log, 0, 0);
	
	return ret;
}
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
/* PDE setup part */
static int prepPDE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *arg)
{
	const struct IVP *ivp = (void *) cl;
	MPT_INTERFACE(logger) *log;
	struct _clientPdeOut ctx;
	int ret;
	
	if ((ret = prepIVP(ivp, arg)) < 0) {
		return ret;
	}
	if (!(ctx.out = ivp->cl.out)) {
		return ret;
	}
	ctx.dat = ivp->sd;
	ctx.state = MPT_ENUM(OutputStateInit);
	
	log = mpt_object_logger((void *) ivp->cl.out);
	mpt_solver_status((void *) ivp->sol, log, outPDE, &ctx);
	
	return ret;
}
/* step operation on solver */
static int stepODE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *arg)
{
	static const char _func[] = "mpt::client<ODE>::step";
	
	const struct IVP *ivp = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log;
	const double *val;
	size_t len;
	int ret, ld, i, state;
	
	log = mpt_object_logger((void *) ivp->cl.out);
	
	if (!ivp->sol || !(dat = ivp->sd)) {
		if (log) mpt_log(log, _func, MPT_FCNLOG(Error), "%s", MPT_tr("client not prepared for step operation"));
		return MPT_ERROR(BadArgument);
	}
	if (!arg) arg = ivp->src;
	ret = mpt_steps_ode(ivp->sol, arg, ivp->sd, log);
	
	state = MPT_ENUM(OutputStateStep);
	if (!ret) state |= MPT_ENUM(OutputStateFini);
	if (ret < 0) state |= MPT_ENUM(OutputStateFail);
	
	ld  = dat->nval;
	val = mpt_data_grid(dat);
	len = dat->val._buf->used / sizeof(*val) / ld;
	
	mpt_output_history(ivp->cl.out, len, val, 1, 0, ld - 1);
	
	for (i = 0; i < ld; i++) {
		if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), i) > 0) {
			continue;
		}
		mpt_output_data(ivp->cl.out, state, i, len, val+i, ld);
	}
	return ret;
}
static int stepPDE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(metatype) *arg)
{
	static const char _func[] = "mpt::client<PDE>::step";
	
	struct IVP *ivp = (void *) cl;
	MPT_INTERFACE(metatype) *src;
	MPT_INTERFACE(logger) *log;
	MPT_SOLVER(IVP) *sol;
	struct _clientPdeOut ctx;
	double end;
	int ret;
	
	if (!(sol = ivp->sol) || !(ctx.dat = ivp->sd)) {
		return -1;
	}
	
	if (!(src = arg) && !(src = ivp->src)) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("no time step source"));
		return MPT_ERROR(BadArgument);
	}
	/* get current target from time source */
	if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
		               MPT_tr("bad value on time argument"));
		return ret;
	}
	if (!ret) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Warning), "%s",
		               MPT_tr("no further values in time source"));
		return 0;
	}
	/* time soure date too low */
	while (end <= ivp->t) {
		mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Info), "%s (%g <= %g)",
		               MPT_tr("skip time value argument"), end, ivp->t);
		
		if ((ret = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &end)) < 0) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
			               MPT_tr("bad value on time argument"));
			return ret;
		}
		if (!ret) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Info), "%s",
			               MPT_tr("no further data in time argument"));
			return 0;
		}
		if (!(ret & MPT_ENUM(ValueConsume))) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
			               MPT_tr("time source is not signaling advancement"));
			return MPT_ERROR(BadOperation);
		}
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
			               MPT_tr("bad value on time argument"));
			return ret;
		}
		if (!ret) {
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Info), "%s",
			               MPT_tr("no further data in time argument"));
			return 0;
		}
	}
	
	log = mpt_object_logger((void *) ivp->cl.out);
	
	while (1) {
		ctx.state = MPT_ENUM(OutputStateStep);
		ret = 1;
		
		ivp->t = end;
		if ((ret = sol->_vptr->step(sol, &ivp->t)) < 0) {
			ctx.state |= MPT_ENUM(OutputStateFail);
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s (t = %g)",
			               MPT_tr("solver step failed"), end);
			break;
		}
		else if (ivp->t < end) {
			mpt_solver_status((void *) sol, log, outPDE, &ctx);
			continue;
		}
		ret = 1;
		if (!src) {
			ctx.state |= MPT_ENUM(OutputStateFini);
			ret = 0;
		}
		else while (1) {
			int curr;
			if ((curr = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &end)) < 0) {
				mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
				               MPT_tr("bad argument on time source"));
				ret = 0;
				break;
			}
			if (!curr) {
				ret = 0;
				ctx.state |= MPT_ENUM(OutputStateFini);
				break;
			}
			if ((curr = src->_vptr->conv(src, 'd', &end)) < 0) {
				mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Error), "%s",
				               MPT_tr("bad argument on time source"));
				ret = 0;
				break;
			}
			if (!curr) {
				ret = 0;
				ctx.state |= MPT_ENUM(OutputStateFini);
				break;
			}
			if (end > ivp->t) {
				break;
			}
			mpt_output_log(ivp->cl.out, _func, MPT_FCNLOG(Info), "%s (%g < %g)",
			               MPT_tr("skip time value argument"), end, ivp->t);
		}
		if ((ctx.out = ivp->cl.out)) {
			mpt_solver_status((void *) sol, log, outPDE, &ctx);
		}
		if (!arg || ret < 1) {
			break;
		}
	}
	return ret;
}
/* clear data of client */
static void clearIVP(MPT_INTERFACE(client) *cl)
{
	const struct IVP *ivp = (void *) cl;
	MPT_STRUCT(path) p = MPT_PATH_INIT;
	MPT_INTERFACE(output) *out;
	MPT_STRUCT(node) *conf;
	
	mpt_path_set(&p, ivp->cfg, -1);
	if ((conf = mpt_config_node(ivp->cfg ? &p : 0))) {
		mpt_node_clear(conf);
	}
	/* close history output */
	if ((out = ivp->cl.out) && mpt_conf_history(out, 0) < 0) {
		mpt_output_log(out, __func__, MPT_FCNLOG(Error), "%s",
		               MPT_tr("unable to close history output"));
	}
}

static MPT_INTERFACE(client) *createSolver(int (*uinit)(), const char *base)
{
	struct IVP *ivp;
	
	if (!uinit) {
		return 0;
	}
	if (!(ivp = malloc(sizeof(*ivp)))) {
		return 0;
	}
	ivp->cl._vptr = 0;
	ivp->cl.out = 0;
	
	(void) memset(&ivp->pr, 0, sizeof(ivp->pr));
	
	ivp->sd = 0;
	ivp->sol = 0;
	ivp->src = 0;
	
	ivp->cfg = base ? strdup(base) : 0;
	
	ivp->t = 0.0;
	
	ivp->uinit = uinit;
	
	return &ivp->cl;
}

static const MPT_INTERFACE_VPTR(client) clientDAE = {
	{ deleteIVP, queryIVP, assignIVP, removeIVP },
	initDAE, prepODE, stepODE, clearIVP
};

/*!
 * \ingroup mptSolver
 * \brief DAE client creation
 * 
 * Create client for solving Differential-Algebraic Equotations.
 * 
 * \param uinit user initialization function
 * 
 * \return DAE client
 */
extern MPT_INTERFACE(client) *mpt_client_dae(int (*uinit)(MPT_SOLVER_STRUCT(daefcn) *, MPT_SOLVER_STRUCT(data) *), const char *base)
{
	MPT_INTERFACE(client) *sol;
	if ((sol = createSolver(uinit, base))) {
		sol->_vptr = &clientDAE;
	}
	return sol;
}

static const MPT_INTERFACE_VPTR(client) clientODE = {
	{ deleteIVP, queryIVP, assignIVP, removeIVP },
	initODE, prepODE, stepODE, clearIVP
};

/*!
 * \ingroup mptSolver
 * \brief ODE client creation
 * 
 * Create client for solving Ordinary Differential Equotations.
 * 
 * \param uinit user initialization function
 * 
 * \return ODE client
 */
extern MPT_INTERFACE(client) *mpt_client_ode(int (*uinit)(MPT_SOLVER_STRUCT(odefcn) *, MPT_SOLVER_STRUCT(data) *), const char *base)
{
	MPT_INTERFACE(client) *sol;
	if ((sol = createSolver(uinit, base))) {
		sol->_vptr = &clientODE;
	}
	return sol;
}

static const MPT_INTERFACE_VPTR(client) ctlPDE = {
	{ deleteIVP, queryIVP, assignIVP, removeIVP },
	initPDE, prepPDE, stepPDE, clearIVP
};

/*!
 * \ingroup mptSolver
 * \brief PDE client creation
 * 
 * Create client for solving Partial Differential Equotations.
 * 
 * \param uinit user initialization function
 * 
 * \return PDE client
 */
extern MPT_INTERFACE(client) *mpt_client_pde(int (*uinit)(MPT_SOLVER_STRUCT(pdefcn) *, MPT_SOLVER_STRUCT(data) *), const char *base)
{
	MPT_INTERFACE(client) *sol;
	if ((sol = createSolver(uinit, base))) {
		sol->_vptr = &ctlPDE;
	}
	return sol;
}

