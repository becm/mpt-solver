/*!
 * create client for IVP problem types
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "node.h"
#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "client.h"

#include "solver.h"

struct IVP {
	MPT_INTERFACE(client)    cl;
	MPT_SOLVER_STRUCT(data) *sd;
	MPT_STRUCT(proxy)        pr;
	MPT_SOLVER_INTERFACE    *sol;
	int (*uinit)(MPT_SOLVER_STRUCT(ivpfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *);
};

/* destruktor */
static int deleteIVP(MPT_INTERFACE(client) *gen)
{
	struct IVP *ivp = (void *) gen;
	MPT_INTERFACE(metatype) *m;
	
	mpt_client_fini(&ivp->cl);
	
	if (ivp->sd) {
		mpt_data_fini(ivp->sd);
		free(ivp->sd);
	}
	if ((m = ivp->pr._mt)) {
		m->_vptr->unref(m);
	}
	free(ivp);
	
	return 0;
}

/* initialisation */
static int initIVP(MPT_INTERFACE(client) *cl, int type)
{
	static const char from[] = "client::init/IVP";
	struct IVP *ivp = (void *) cl;
	MPT_INTERFACE(logger) *log = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out);
	MPT_STRUCT(node) *node = mpt_node_find(cl->conf, "solver", 1);
	const char *conf;
	int ret = 1;
	
	if (!(conf = node ? mpt_node_data(node, 0) : 0)) {
		if (!ivp->sol) {
			(void) mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("missing solver description"));
			return -3;
		}
		ret = 0;
	}
	else {
		const char *a = mpt_solver_alias(conf);
		if (mpt_library_bind(&ivp->pr, a ? a : conf, log) < 0) {
			return -1;
		}
	}
	if (!(ivp->sol = ivp->pr._mt->_vptr->typecast(ivp->pr._mt, MPT_ENUM(TypeSolver)))) {
		(void) mpt_log(log, 0, MPT_ENUM(LogError), "%s: %s", MPT_tr("solver"), conf);
		return -2;
	}
	conf = mpt_meta_typename((void *) ivp->sol);
	
	if (mpt_solver_check(ivp->sol, type) < 0) {
		if (!conf) conf = "solver";
		switch (type) {
		  case MPT_SOLVER_ENUM(ODE): (void) mpt_log(log, from, MPT_ENUM(LogError), "%s: %s", MPT_tr("unable to handle ODE problem")); break;
		  case MPT_SOLVER_ENUM(DAE): (void) mpt_log(log, from, MPT_ENUM(LogError), "%s: %s", MPT_tr("unable to handle DAE problem")); break;
		  case MPT_SOLVER_ENUM(PDE): (void) mpt_log(log, from, MPT_ENUM(LogError), "%s: %s", MPT_tr("unable to handle PDE problem")); break;
		  default:;
		}
		return -3;
	}
	if (conf) {
		(void) mpt_log(log, 0, MPT_ENUM(LogMessage), "%s: %s", MPT_tr("solver"), conf);
	}
	if (!ivp->sd) {
		if (!(ivp->sd = malloc(sizeof(*ivp->sd)))) {
			(void) mpt_log(log, from, MPT_ENUM(LogCritical), "%s", MPT_tr("no memory for data"));
			return -2;
		}
		mpt_data_init(ivp->sd);
	}
	mpt_conf_graphic(cl->out, cl->conf->children);
	
	return ret;
}
static int initDAE(MPT_INTERFACE(client) *cl)
{
	return initIVP(cl, MPT_SOLVER_ENUM(DAE));
}
static int initODE(MPT_INTERFACE(client) *cl)
{
	return initIVP(cl, MPT_SOLVER_ENUM(ODE));
}
static int initPDE(MPT_INTERFACE(client) *cl)
{
	return initIVP(cl, MPT_SOLVER_ENUM(PDE));
}
/* combined IVP setup part */
static int prepIVP(MPT_SOLVER_INTERFACE *gen, MPT_INTERFACE(source) *arg, MPT_SOLVER_STRUCT(ivppar) *ivp, double *val, double *grid, const MPT_STRUCT(node) *conf, MPT_INTERFACE(output) *out)
{
	const MPT_INTERFACE_VPTR(Ivp) *ictl;
	MPT_INTERFACE(logger) *log = MPT_LOGGER((MPT_INTERFACE(metatype) *) out);
	
	if (mpt_meta_set((void *) gen, 0, "iid", ivp) <= 0) {
		mpt_log(log, "client::prep/IVP", MPT_ENUM(LogError), "%s", MPT_tr("unable to save problem parameter to solver"));
		return -3;
	}
	mpt_solver_param((void *) gen, conf, arg, log);
	
	ictl = (void *) gen->_vptr;
	if (ictl->step(gen, val, 0, grid) < 0) {
		mpt_log(log, "client::prep/IVP", MPT_ENUM(LogError), "%s", MPT_tr("preparing solver backend failed"));
		return -3;
	}
	if (!out) {
		return 0;
	}
	mpt_solver_info  (gen, log);
	mpt_log(log, 0, MPT_ENUM(LogMessage), "");
	mpt_solver_status(gen, log);
	
	return 0;
}
static int prepODE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(source) *arg)
{
	const struct IVP *ivp = (void *) cl;
	const MPT_INTERFACE_VPTR(Ivp) *ictl;
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out);
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(ivppar) ipar;
	double *grid;
	
	if (!(gen = ivp->sol)) {
		mpt_log(log, "client::prep/ODE", MPT_ENUM(LogError), "%s", MPT_tr("no solver assigned"));
		return -1;
	}
	if (!(dat = ivp->sd)) {
		mpt_log(log, "client::prep/ODE", MPT_ENUM(LogError), "%s", MPT_tr("missing solver data"));
		return -1;
	}
	if (mpt_conf_history(cl->out, cl->conf->children) < 0) {
		return -1;
	}
	/* clear generated data */
	mpt_data_clear(dat);
	
	/* read ODE parameter */
	if ((ipar.pint = mpt_conf_ode(dat, cl->conf->children, log)) < 0) {
		return -2;
	}
	ictl = (void *) gen->_vptr;
	ufcn = ictl->ufcn(gen);
	
	/* call user init function */
	if ((ipar.neqs = ivp->uinit(ufcn, dat, cl->out)) < 0) {
		return ipar.neqs;
	}
	if (!(ipar.neqs)) {
		mpt_log(log, "client::prep/ODE", MPT_ENUM(LogError), "%s", MPT_tr("user function must return equotation count"));
		return -1;
	}
	if (ipar.neqs > ipar.pint) {
		mpt_log(log, "client::prep/ODE", MPT_ENUM(LogError), "%s: %i..%i", MPT_tr("uninitialized initial values"), ipar.pint+1, ipar.neqs);
	}
	dat->nval = 1 + ipar.neqs;
	
	if (!(grid = mpt_data_grid(dat, 0))) {
		mpt_log(log, "client::prep/ODE", MPT_ENUM(LogError), "%s", MPT_tr("unable to reserve initial data"));
		return -1;
	}
	/* trim to state data size */
	dat->val._buf->used = dat->nval * sizeof(double);
	
	ipar.pint = 0;
	ipar.last = grid[0];
	
	return prepIVP(gen, arg, &ipar, grid+1, 0, cl->conf->children, cl->out);
}
/* PDE setup part */
static int prepPDE(MPT_INTERFACE(client) *cl, MPT_INTERFACE(source) *arg)
{
	const struct IVP *ivp = (void *) cl;
	const MPT_INTERFACE_VPTR(Ivp) *ictl;
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(logger) *log = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out);
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(ivppar) ipar;
	double	*grid = 0;
	int	len;
	
	if (!(gen = ivp->sol)) {
		mpt_log(log, "client::prep/PDE", MPT_ENUM(LogError), "%s", MPT_tr("no solver assigned"));
		return -1;
	}
	if (!(dat = ivp->sd)) {
		mpt_log(log, "client::prep/PDE", MPT_ENUM(LogError), "%s", MPT_tr("missing solver data"));
		return -1;
	}
	if (mpt_conf_history(cl->out, cl->conf->children) < 0) {
		return -1;
	}
	/* clear generated data */
	mpt_data_clear(dat);
	
	/* PDE configuration */
	if (mpt_conf_pde(dat, cl->conf->children, log) < 0) {
		return -2;
	}
	ictl = (void *) gen->_vptr;
	ufcn = ictl->ufcn(gen);
	
	/* call user init function */
	if ((ipar.neqs = ivp->uinit(ufcn, dat, cl->out)) < 0) {
		return -3;
	}
	/* no grid requested */
	if (!(len = dat->nval)) {
		grid = mpt_data_grid(dat, ipar.neqs);
	}
	/* read profile data */
	if ((len = dat->nval) <= 0) {
		len = -dat->nval;
		if (mpt_conf_profile(&dat->val, len, ipar.neqs, mpt_node_next(cl->conf->children, "profile"), log) < 0) {
			return -3;
		}
		dat->nval = len;
	}
	if (!grid) {
		grid = mpt_data_grid(dat, ipar.neqs);
	}
	ipar.pint = dat->nval - 1;
	
	if (isnan(ipar.last = mpt_iterator_curr(dat->iter))) {
		mpt_log(log, "client::prep/PDE", MPT_ENUM(LogError), "%s", MPT_tr("bad initial iterator value"));
		return -3;
	}
	return prepIVP(gen, arg, &ipar, grid+len, grid, cl->conf->children, cl->out);
}
/* step operation on solver */
static int stepODE(MPT_INTERFACE(client) *cl)
{
	const struct IVP *ivp = (void *) cl;
	
	if (!ivp->sol || !ivp->sd) {
		return -1;
	}
	return mpt_step_ode(ivp->sol, ivp->sd, MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out));
}
static int stepPDE(MPT_INTERFACE(client) *cl)
{
	const struct IVP *ivp = (void *) cl;
	
	if (!ivp->sol || !ivp->sd) {
		return -1;
	}
	return mpt_step_pde(ivp->sol, ivp->sd, MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out));
}
/* clear data of client */
static void clearIVP(MPT_INTERFACE(client) *cl)
{
	if (cl->conf) mpt_node_clear(cl->conf);
	
	/* close history output */
	if (cl->out && mpt_conf_history(cl->out, 0) < 0) {
		mpt_output_log(cl->out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("unable to close history output"));
	}
}
/* output for IVP solvers */
static int outPDE(const MPT_INTERFACE(client) *cl, int state)
{
	const struct IVP *ivp = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(output) *out;
	MPT_SOLVER_INTERFACE *gen;
	double	*x, *y, t;
	int	i, len, ld;
	
	if (!state || !(out = cl->out)) {
		return -1;
	}
	if (!(dat = ivp->sd) || !(x = mpt_data_grid(dat, 0))) {
		return 0;
	}
	y  = x + (len = dat->nval);
	ld = (dat->val._buf->used / sizeof(*x)) / len;
	
	if (!(--ld)) {
		mpt_output_log(cl->out, "client::out/PDE", MPT_ENUM(LogError), "%s", MPT_tr("missing PDE values"));
		return -2;
	}
	/* time information */
	if ((gen = ivp->sol)
	    && ((MPT_INTERFACE_VPTR(Ivp) *) gen->_vptr)->step(gen, 0, &t, 0) >= 0) {
		struct {
			MPT_STRUCT(msgtype) mt;
			MPT_STRUCT(msgbind) bnd;
		} hdr;
		
		hdr.mt.cmd   = MPT_ENUM(MessageValRaw);
		hdr.mt.arg   = 0;
		hdr.bnd.dim  = state & 0xff; /* special data state as dimension info */
		hdr.bnd.type = MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(t);
		
		/* push parameter data */
		out->_vptr->push(out, sizeof(hdr), &hdr);
		out->_vptr->push(out, sizeof(t), &t);
		out->_vptr->push(out, 0, 0);
	}
	
	mpt_output_history(out, len, x, 1, y, ld);
	
	if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), 0) <= 0) {
		mpt_output_data(out, state, 0, len, x, 1);
	}
	for (i = 1; i <= ld; i++) {
		if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), i) > 0) continue;
		mpt_output_data(out, state, i, len, y++, ld);
	}
	return len;
}

static int outODE(const MPT_INTERFACE(client) *cl, int state)
{
	const struct IVP *ivp = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(output) *out;
	double	*val;
	int	i, ld, len;
	
	/* filter initial output */
	if (!(state & MPT_ENUM(OutputStateFini))) {
		return 0;
	}
	if (!(out = cl->out)) {
		return -1;
	}
	if (!(dat = ivp->sd) || !(val = mpt_data_grid(dat, 0))) {
		return 0;
	}
	if ((ld = dat->nval) <= 0) {
		ld  = -ld;
		len = 1;
	} else {
		len = (dat->val._buf->used / sizeof(*val)) / ld;
	}
	
	mpt_output_history(out, len, val, 1, 0, ld-1);
	
	for (i = 0; i < ld; i++) {
		if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), i) > 0) continue;
		mpt_output_data(out, state, i, len, val++, ld);
	}
	
	return ld;
}

/* report for IVP solvers */
static int reportIVP(const MPT_INTERFACE(client) *cl, MPT_INTERFACE(logger) *out)
{
	const struct IVP *ivp = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	
	if (!out && !(out = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out))) {
		return 0;
	}
	if (!ivp->sol) {
		return -1;
	}
	if ((dat = ivp->sd)) {
		mpt_client_report(out, ivp->sol, &dat->ru_usr, &dat->ru_sys);
		return 2;
	}
	mpt_client_report(out, ivp->sol, 0, 0);
	return 1;
}

static MPT_INTERFACE(client) *createSolver(int (*uinit)(MPT_SOLVER_STRUCT(ivpfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *))
{
	struct IVP *ivp;
	
	if (!(ivp = malloc(sizeof(*ivp)))) {
		return 0;
	}
	mpt_client_init(&ivp->cl);
	
	ivp->uinit = uinit;
	
	(void) memset(&ivp->sol, 0, sizeof(ivp->sol));
	
	return &ivp->cl;
}

static const MPT_INTERFACE_VPTR(client) ctlDAE = {
	deleteIVP,
	initDAE, prepODE, stepODE, clearIVP,
	outODE, reportIVP
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
extern MPT_INTERFACE(client) *mpt_client_dae(int (*uinit)(MPT_SOLVER_STRUCT(ivpfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *))
{
	MPT_INTERFACE(client) *sol;
	if ((sol = createSolver(uinit))) {
		sol->_vptr = &ctlDAE;
	}
	return sol;
}

static const MPT_INTERFACE_VPTR(client) ctlODE = {
	deleteIVP,
	initODE, prepODE, stepODE, clearIVP,
	outODE, reportIVP
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
extern MPT_INTERFACE(client) *mpt_client_ode(int (*uinit)(MPT_SOLVER_STRUCT(ivpfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *))
{
	MPT_INTERFACE(client) *sol;
	if ((sol = createSolver(uinit))) {
		sol->_vptr = &ctlODE;
	}
	return sol;
}

static const MPT_INTERFACE_VPTR(client) ctlPDE = {
	deleteIVP,
	initPDE, prepPDE, stepPDE, clearIVP,
	outPDE, reportIVP
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
extern MPT_INTERFACE(client) *mpt_client_pde(int (*uinit)(MPT_SOLVER_STRUCT(ivpfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *))
{
	MPT_INTERFACE(client) *sol;
	if ((sol = createSolver(uinit))) {
		sol->_vptr = &ctlPDE;
	}
	return sol;
}

