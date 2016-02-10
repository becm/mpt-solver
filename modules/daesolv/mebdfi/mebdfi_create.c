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

static int meReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_mebdfi_report((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1), what, out, data);
}

static int meStep(MPT_SOLVER(IVP) *sol, double *tend)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (sol + 1);
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
static void *meFcn(MPT_SOLVER(IVP) *sol, int type)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (sol + 1);
	MPT_SOLVER_IVP_STRUCT(functions) *fcn = (void *) (me + 1);
	
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return me->ivp.pint ? 0 : &fcn->dae;
	  case MPT_SOLVER_ENUM(PDE): if (me->ivp.pint) return 0; fcn->dae.mas = 0; fcn->dae.jac = 0;
	  case MPT_SOLVER_ENUM(IVP): return fcn;
	  default: return 0;
	}
	if (me->ivp.pint) {
		return 0;
	}
	fcn->dae.mas = 0;
	return &fcn->dae;
}
static double *meState(MPT_SOLVER(IVP) *sol)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (sol + 1);
	return me->y;
}

static const MPT_INTERFACE_VPTR(solver_ivp) mebdfiCtl = {
	{ { meFini, meAddref, meGet, meSet }, meReport },
	meStep,
	meFcn,
	meState
};

extern MPT_SOLVER(IVP) *mpt_mebdfi_create()
{
	MPT_SOLVER(IVP) *sol;
	MPT_SOLVER_STRUCT(mebdfi) *md;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*md) + sizeof(MPT_SOLVER_IVP_STRUCT(functions))))) {
		return 0;
	}
	md = (MPT_SOLVER_STRUCT(mebdfi) *) (sol+1);
	mpt_mebdfi_init(md);
	
	MPT_IVPFCN_INIT(md + 1)->dae.param = &md;
	
	sol->_vptr = &mebdfiCtl;
	
	return sol;
}

