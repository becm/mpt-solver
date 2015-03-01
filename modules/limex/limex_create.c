/*!
 * generic solver interface for LIMEX
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "limex.h"

static MPT_SOLVER_STRUCT(limex)  lxGlob;
static MPT_SOLVER_STRUCT(ivpfcn) lxGlobFcn;

extern MPT_SOLVER_STRUCT(limex) *mpt_limex_global()
{
	if (lxGlob.ufcn) return &lxGlob;
	
	mpt_limex_init(&lxGlob);
	
	lxGlob.ufcn = MPT_IVPFCN_INIT(&lxGlobFcn);
	lxGlobFcn.param = &lxGlob.ivp;
	
	return &lxGlob;
}

extern int mpt_limex_step_(MPT_SOLVER_STRUCT(limex) *lx, double *u, double *end)
{
	int err;
	if (lxGlob.iopt[15] < 0 || !end) {
		if (lx->ufcn && mpt_limex_ufcn(lx, lx->ufcn) < 0) return -1;
		if ((err = mpt_limex_prepare(lx, lx->ivp.neqs, lx->ivp.pint)) < 0 || !end)
			return err;
	}
	err = u ? mpt_limex_step(lx, u, *end) : 0;
	*end = lx->ivp.last;
	return err;
}

static int lxFini(MPT_INTERFACE(metatype) *gen)
{ (void) gen; mpt_limex_fini(&lxGlob); return 0; }

static MPT_INTERFACE(metatype) *lxAddref()
{ return 0; }

static int lxProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ (void) gen; return mpt_limex_property(&lxGlob, pr, src); }

static void *lxCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int lxReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ (void) gen; return mpt_limex_report(&lxGlob, what, out, data); }

static int lxStep(MPT_SOLVER_INTERFACE *gen, double *u, double *tend, double *x)
{ (void) gen; (void) x; return mpt_limex_step_(&lxGlob, u, tend); }

static MPT_SOLVER_STRUCT(ivpfcn) *lxFcn(const MPT_SOLVER_INTERFACE *gen)
{ (void) gen; return &lxGlobFcn; }

static const MPT_INTERFACE_VPTR(Ivp) limexCtl = {
	{ { lxFini, lxAddref, lxProperty, lxCast }, lxReport },
	lxStep,
	lxFcn
};

extern MPT_SOLVER_INTERFACE *mpt_limex_create()
{
	static const MPT_SOLVER_INTERFACE lxGlobControl = { &limexCtl.gen };
	if (lxGlob.ufcn) return 0;
	mpt_limex_global();
	return (MPT_SOLVER_INTERFACE *) &lxGlobControl;
}
