/*!
 * generic solver interface for LIMEX
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "limex.h"

static MPT_SOLVER_STRUCT(limex)  lxGlob;
static MPT_SOLVER_TYPE(ivpfcn) lxGlobFcn;

extern MPT_SOLVER_STRUCT(limex) *mpt_limex_global()
{
	if (lxGlob.ufcn) return &lxGlob;
	
	mpt_limex_init(&lxGlob);
	
	lxGlob.ufcn = &MPT_IVPFCN_INIT(&lxGlobFcn)->dae;
	lxGlobFcn.dae.param = &lxGlob.ivp;
	
	return &lxGlob;
}

static void lxFini(MPT_INTERFACE(object) *gen)
{
	(void) gen;
	mpt_limex_fini(&lxGlob);
}
static uintptr_t lxAddref()
{
	return 0;
}
static int lxGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	(void) gen;
	return mpt_limex_get(&lxGlob, pr);
}
static int lxSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	(void) gen;
	return mpt_limex_set(&lxGlob, pr, src);
}

static int lxReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	(void) gen;
	return mpt_limex_report(&lxGlob, what, out, data);
}

static int lxStep(MPT_SOLVER_INTERFACE *gen, double *tend)
{
	int ret;
	(void) gen;
	
	if (!tend) {
		if (!lxGlob.fcn && (ret = mpt_limex_ufcn(&lxGlob, &lxGlobFcn.dae))) {
			return ret;
		}
		return mpt_limex_prepare(&lxGlob);
	}
	ret = mpt_limex_step(&lxGlob, *tend);
	*tend = lxGlob.t;
	return ret;
}
static void *lxFcn(const MPT_SOLVER_INTERFACE *gen, int type)
{
	(void) gen;
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return lxGlob.ivp.pint ? 0 : &lxGlobFcn.dae;
	  case MPT_SOLVER_ENUM(PDE): return lxGlob.ivp.pint ? &lxGlobFcn.pde : 0;
	  default: return 0;
	}
	if (lxGlob.ivp.pint) {
		return 0;
	}
	lxGlobFcn.dae.mas = 0;
	return &lxGlobFcn.ode;
}
static double *lxState(MPT_SOLVER_INTERFACE *gen)
{
	(void) gen;
	return lxGlob.y;
}

static const MPT_INTERFACE_VPTR(Ivp) limexCtl = {
	{ { lxFini, lxAddref, lxGet, lxSet }, lxReport },
	lxStep,
	lxFcn,
	lxState
};

extern MPT_SOLVER_INTERFACE *mpt_limex_create()
{
	static const MPT_SOLVER_INTERFACE lxGlobControl = { &limexCtl.gen };
	if (lxGlob.ufcn) return 0;
	mpt_limex_global();
	return (MPT_SOLVER_INTERFACE *) &lxGlobControl;
}
