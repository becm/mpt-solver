/*!
 * create client for solving nonlinear systems.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "node.h"
#include "message.h"
#include "array.h"

#include "values.h"
#include "output.h"

#include "client.h"

#include "solver.h"

struct NLS {
	MPT_INTERFACE(client)    cl;
	MPT_SOLVER_STRUCT(data) *sd;
	MPT_STRUCT(proxy)        pr;
	MPT_SOLVER_INTERFACE    *sol;
	int (*uinit)(MPT_SOLVER_STRUCT(nlsfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *);
};

static int deleteNLS(MPT_INTERFACE(client) *gen)
{
	struct NLS *nls = (void *) gen;
	MPT_INTERFACE(metatype) *m;
	
	mpt_client_fini(&nls->cl);
	
	if (nls->sd) {
		mpt_data_fini(nls->sd);
		free(nls->sd);
	}
	if ((m = nls->pr._mt)) {
		m->_vptr->unref(m);
	}
	free(nls);
	
	return 0;
}

static int initNLS(MPT_INTERFACE(client) *cl)
{
	static const char from[] = "mpt::client::init/NLS";
	struct NLS *nls = (void *) cl;
	MPT_STRUCT(node) *node = mpt_node_find(cl->conf, "solver", 1);
	MPT_INTERFACE(logger) *log = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out);
	const char *conf;
	int ret = 1;
	
	if (!(conf = node ? mpt_node_data(node, 0) : 0)) {
		if (!nls->sol) {
			(void) mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("missing solver description"));
			return -3;
		}
		ret = 0;
	}
	else {
		const char *a = mpt_solver_alias(conf);
		if (mpt_library_bind(&nls->pr, a ? a : conf, log) < 0) {
			return -1;
		}
	}
	if (!(nls->sol = nls->pr._mt->_vptr->typecast(nls->pr._mt, MPT_ENUM(TypeSolver)))) {
		(void) mpt_log(log, 0, MPT_ENUM(LogError), "%s: %s", MPT_tr("solver"), conf);
		return -2;
	}
	conf = mpt_meta_typename((void *) nls->sol);
	
	if (mpt_solver_check(nls->sol, -MPT_SOLVER_ENUM(CapableNls)) < 0) {
		(void) mpt_log(log, from, MPT_ENUM(LogError), "%s: %s",
		               conf ? conf : "solver",
		               MPT_tr("unable to handle NLS problem"));
		return -3;
	}
	if (conf) {
		(void) mpt_log(log, 0, MPT_ENUM(LogMessage), "%s: %s", MPT_tr("solver"), conf);
	}
	if (!nls->sd) {
		if (!(nls->sd = malloc(sizeof(*nls->sd)))) {
			(void) mpt_log(log, from, MPT_ENUM(LogCritical), "%s", MPT_tr("no memory for data"));
			return -2;
		}
		mpt_data_init(nls->sd);
	}
	mpt_conf_graphic(cl->out, cl->conf->children);
	
	return ret;
}

static int prepNLS(MPT_INTERFACE(client) *cl, MPT_INTERFACE(source) *args)
{
	static const char from[] = "mpt::client::prep/NLS";
	const struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_SOLVER_STRUCT(nlsfcn) *ufcn;
	MPT_SOLVER_STRUCT(nlspar) par;
	MPT_INTERFACE(logger) *log = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out);
	MPT_SOLVER_INTERFACE *gen;
	const MPT_INTERFACE_VPTR(Nls) *nctl;
	int nval;
	
	if (!(gen = nls->sol)) {
		mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("no solver assigned"));
		return -1;
	}
	if (!(dat = nls->sd)) {
		mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("missing solver data"));
		return -1;
	}
	if (mpt_conf_history(cl->out, cl->conf->children) < 0) {
		return -1;
	}
	/* clear generated data */
	mpt_data_clear(dat);
	
	nctl = (void *) gen->_vptr;
	ufcn = nctl->ufcn(gen);
	
	if ((par.nval = mpt_conf_nls(dat, cl->conf->children, log)) < 0) {
		mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("solver preparation failed"));
		return -2;
	}
	/* call user init function */
	if ((nval = nls->uinit(ufcn, dat, cl->out)) < 0) {
		return nval;
	}
	if (dat->npar < nval) {
		mpt_log(log, from, MPT_ENUM(LogError), "%s: %i < %i", MPT_tr("not enough parameter"), dat->npar, nval);
		return -2;
	}
	
	if (nval) par.nval = nval;
	par.nres = dat->nval;
	
	if (mpt_meta_set((void *) gen, 0, "ii", &par) <= 0) {
		mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("unable to save problem dimensions to solver"));
		return -3;
	}
	mpt_solver_param((void *) gen, cl->conf->children, args, log);
	
	if (nctl->step(gen, mpt_data_param(dat), 0) < 0) {
		mpt_log(log, from, MPT_ENUM(LogError), "%s", MPT_tr("preparing solver backend failed"));
		return -3;
	}
	if (!log) {
		return 0;
	}
	mpt_solver_info  (gen, log);
	mpt_log(log, 0, MPT_ENUM(LogMessage), "");
	mpt_solver_status(gen, log);
	
	return 0;
}

static int stepNLS(MPT_INTERFACE(client) *cl)
{
	const struct NLS *nls = (void *) cl;
	MPT_SOLVER_INTERFACE *gen;
	if (!(gen = (void *) nls->sol) || !nls->sd) {
		return -1;
	}
	return mpt_step_nls(gen, nls->sd, MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out));
}

static void clearNLS(MPT_INTERFACE(client) *cl)
{
	if (cl->conf) mpt_node_clear(cl->conf);
	
	/* close history output */
	if (cl->out && mpt_conf_history(cl->out, 0) < 0) {
		mpt_output_log(cl->out, __func__, MPT_ENUM(LogError), "%s", MPT_tr("unable to close history output"));
	}
}
/* output for NLS solvers */
static int outNLS(const MPT_INTERFACE(client) *cl, int state)
{
	const struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	MPT_INTERFACE(output) *out;
	const double *res, *p, *ex;
	int i, len, ld;
	
	if (!state || !(out = cl->out) || !(dat = nls->sd)) {
		return -1;
	}
	if (!(p = mpt_data_param(dat))) {
		return -1;
	}
	len = dat->nval;
	res = mpt_data_grid(dat, 0);
	ex = res + len;
	ld = ((dat->val._buf->used - len) / sizeof(*res)) / len;
	
	if (state & MPT_ENUM(OutputStateInit)) {
		for (i = 0; i < ld; i++) {
			if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), i+1) > 0) {
				continue;
			}
			mpt_output_data(out, state, i, len, ex+i, ld);
		}
	}
	/* output parameter and residuals */
	if (state & (MPT_ENUM(OutputStateStep) | MPT_ENUM(OutputStateFini))) {
		struct {
			MPT_STRUCT(msgtype) mt;
			MPT_STRUCT(msgbind) bnd;
		} hdr;
		
		hdr.mt.cmd   = MPT_ENUM(MessageValRaw);
		hdr.mt.arg   = 0; /* indicate special data */
		hdr.bnd.dim  = state & 0xff; /* repurpose dimension as special data state */
		hdr.bnd.type = MPT_ENUM(ByteOrderNative) | MPT_ENUM(ValuesFloat) | sizeof(*p);
		
		/* push parameter data */
		out->_vptr->push(out, sizeof(hdr), &hdr);
		out->_vptr->push(out, dat->npar * sizeof(*p), p);
		out->_vptr->push(out, 0, 0);
		
		if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), 0) <= 0) {
			mpt_output_data(out, state, 0, len, res, 1);
		}
		mpt_output_history(out, len, res, 1, ex, ld);
		
	}
	/* output residuals and user data */
	if (state & MPT_ENUM(OutputStateFini)) {
		MPT_STRUCT(node) *pbase;
		
		if ((pbase = cl->conf) && (pbase = mpt_node_next(pbase->children, "param"))) {
			pbase = pbase->children;
		}
		for (i = 0; i < dat->npar; ++i) {
			const char *name, *desc = MPT_tr("parameter");
			if (pbase && (name = mpt_node_ident(pbase))) {
				mpt_output_log(out, 0, MPT_ENUM(LogMessage), "%s %2d: %16g (%s)", desc, i+1, p[i], name);
			} else {
				mpt_output_log(out, 0, MPT_ENUM(LogMessage), "%s %2d: %16g",      desc, i+1, p[i]);
			}
		}
		for (i = 0; i < ld; i++) {
			if (mpt_bitmap_get(dat->mask, sizeof(dat->mask), i+1) > 0) {
				continue;
			}
			mpt_output_data(out, state, i, len, ex+i, ld);
		}
	}
	
	return len;
}

/* report for NLS solvers */
static int reportNLS(const MPT_INTERFACE(client) *cl, MPT_INTERFACE(logger) *out)
{
	const struct NLS *nls = (void *) cl;
	MPT_SOLVER_STRUCT(data) *dat;
	
	if (!out && !(out = MPT_LOGGER((MPT_INTERFACE(metatype) *) cl->out))) {
		return 0;
	}
	if (!nls->sol) {
		return -2;
	}
	if ((dat = nls->sd)) {
		mpt_solver_statistics(nls->sol, out, &dat->ru_usr, &dat->ru_sys);
		return 2;
	}
	mpt_solver_statistics(nls->sol, out, 0, 0);
	return 1;
}

static MPT_INTERFACE_VPTR(client) ctlNLS = {
	deleteNLS,
	initNLS, prepNLS, stepNLS, clearNLS,
	outNLS, reportNLS
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
extern MPT_INTERFACE(client) *mpt_client_nls(int (*uinit)(MPT_SOLVER_STRUCT(nlsfcn) *, MPT_SOLVER_STRUCT(data) *, MPT_INTERFACE(output) *))
{
	struct NLS *nls;
	
	if (!(nls = malloc(sizeof(*nls)))) {
		return 0;
	}
	mpt_client_init(&nls->cl);
	nls->sd = 0;
	(void) memset(&nls->pr, 0, sizeof(nls->pr));
	nls->sol = 0;
	nls->uinit = uinit;
	
	nls->cl._vptr = &ctlNLS;
	return &nls->cl;
}

