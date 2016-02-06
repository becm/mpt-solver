/*!
 * generic solver interface for MEBDFI
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mebdfi.h"

static void meFini(MPT_INTERFACE(object) *gen)
{
	mpt_mebdfi_fini((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1));
	free(gen);
}
static uintptr_t meAddref()
{
	return 0;
}
static int meGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return mpt_mebdfi_get((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1), pr);
}
static int meSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_mebdfi_set((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1), pr, src);
}

static int meReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_mebdfi_report((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1), what, out, data);
}

static int meStep(MPT_SOLVER_INTERFACE *gen, double *tend)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (gen + 1);
	int err;
	
	if (!tend) {
		if (!me->fcn && (err = mpt_mebdfi_ufcn(me, (void *) (me + 1))) < 0) {
			return err;
		}
		return mpt_mebdfi_prepare(me);
	}
	err = mpt_mebdfi_step(me, *tend);
	*tend = me->t;
	return err;
}
static void *meFcn(MPT_SOLVER_INTERFACE *gen, int type)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (gen + 1);
	MPT_SOLVER_TYPE(ivpfcn) *ivp = (void *) (me + 1);
	
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return me->ivp.pint ? 0 : &ivp->dae;
	  case MPT_SOLVER_ENUM(PDE): return me->ivp.pint ? &ivp->pde : 0;
	  default: return 0;
	}
	if (me->ivp.pint) {
		return 0;
	}
	ivp->dae.mas = 0;
	return &ivp->ode;
}
static double *meState(MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (gen + 1);
	return me->y;
}

static const MPT_INTERFACE_VPTR(Ivp) mebdfiCtl = {
	{ { meFini, meAddref, meGet, meSet }, meReport },
	meStep,
	meFcn,
	meState
};

extern MPT_SOLVER_INTERFACE *mpt_mebdfi_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(mebdfi) *md;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*md) + sizeof(MPT_SOLVER_TYPE(ivpfcn))))) {
		return 0;
	}
	md = (MPT_SOLVER_STRUCT(mebdfi) *) (gen+1);
	mpt_mebdfi_init(md);
	
	MPT_IVPFCN_INIT(md + 1)->ode.param = &md->ivp;
	
	gen->_vptr = &mebdfiCtl.gen;
	
	return gen;
}

