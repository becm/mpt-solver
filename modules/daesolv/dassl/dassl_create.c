/*!
 * generic solver interface for dDASSL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "dassl.h"

static void ddFini(MPT_INTERFACE(object) *gen)
{
	mpt_dassl_fini((MPT_SOLVER_STRUCT(dassl *)) (gen+1));
	free(gen);
}
static uintptr_t ddAddref()
{
	return 0;
}
static int ddGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return mpt_dassl_get((void *) (gen+1), pr);
}
static int ddSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_dassl_set((void *) (gen+1), pr, src);
}

static int ddReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_dassl_report((MPT_SOLVER_STRUCT(dassl *)) (gen+1), what, out, data);
}

static int dasslStep(MPT_SOLVER_INTERFACE *gen, double *tend)
{
	MPT_SOLVER_STRUCT(dassl) *da = (void *) (gen + 1);
	int err;
	
	if (!tend) {
		if (!da->fcn && (err = mpt_dassl_ufcn(da, (void *)(da + 1))) < 0) {
			return err;
		}
		return mpt_dassl_prepare(da);
	}
	err = mpt_dassl_step(da, *tend);
	*tend = da->t;
	return err;
}
static void *ddFcn(MPT_SOLVER_INTERFACE *gen, int type)
{
	MPT_SOLVER_STRUCT(dassl) *da = (void *) (gen + 1);
	MPT_SOLVER_TYPE(ivpfcn) *ivp = (void *) (da + 1);
	
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return da->ivp.pint ? 0 : &ivp->dae;
	  case MPT_SOLVER_ENUM(PDE): return da->ivp.pint ? &ivp->pde : 0;
	  default: return 0;
	}
	if (da->ivp.pint) {
		return 0;
	}
	ivp->dae.mas = 0;
	return &ivp->ode;
}
static double *ddState(MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(dassl *) da = (void *) (gen + 1);
	return da->y;
}
static const MPT_INTERFACE_VPTR(Ivp) dasslCtl = {
	{ { ddFini, ddAddref, ddGet, ddSet }, ddReport },
	dasslStep,
	ddFcn,
	ddState
};

extern MPT_SOLVER_INTERFACE *mpt_dassl_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(dassl) *da;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*da) + sizeof(MPT_SOLVER_TYPE(ivpfcn))))) {
		return 0;
	}
	da = (MPT_SOLVER_STRUCT(dassl *)) (gen+1);
	mpt_dassl_init(da);
	
	gen->_vptr = &dasslCtl.gen;
	
	MPT_IVPFCN_INIT(da+1)->dae.param = &da->ivp;
	
	return gen;
}
