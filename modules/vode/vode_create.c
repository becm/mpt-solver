/*!
 * generic solver interface for dVODE
 */

#include <stdlib.h>
#include <string.h>

#include "vode.h"

static void vdFini(MPT_INTERFACE(unrefable) *gen)
{
	mpt_vode_fini((MPT_SOLVER_STRUCT(vode) *) (gen+1));
	free(gen);
}
static uintptr_t vdAddref()
{
	return 0;
}
static int vdGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return mpt_vode_get((MPT_SOLVER_STRUCT(vode) *) (gen+1), pr);
}
static int vdSet(MPT_INTERFACE(object) *gen, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	return mpt_vode_set((MPT_SOLVER_STRUCT(vode) *) (gen+1), pr, src);
}

static int vdReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_vode_report((MPT_SOLVER_STRUCT(vode) *) (gen+1), what, out, data);
}

static int vdStep(MPT_SOLVER(IVP) *sol, double *tend)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (sol+1);
	int ret;
	
	if (!tend) {
		if (!vd->fcn && (ret = mpt_vode_ufcn(vd, (void *) (vd+1))) < 0) {
			return ret;
		}
		return mpt_vode_prepare(vd);
	}
	ret = mpt_vode_step(vd, *tend);
	*tend = vd->t;
	return ret;
}
static void *vdFcn(MPT_SOLVER(IVP) *sol, int type)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (sol+1);
	MPT_SOLVER_IVP_STRUCT(functions) *fcn = (void *) (vd+1);
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(PDE): if (!vd->ivp.pint) return 0; fcn->dae.jac = 0; fcn->dae.mas = 0;
	  case MPT_SOLVER_ENUM(IVP): return fcn;
	  default: return 0;
	}
	if (vd->ivp.pint) {
		return 0;
	}
	fcn->dae.mas = 0;
	return &fcn->dae;
}
static double *vdInitState(MPT_SOLVER(IVP) *sol)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (sol+1);
	return vd->y;
}
static const MPT_INTERFACE_VPTR(solver_ivp) vodeCtl = {
	{ { { vdFini }, vdAddref, vdGet, vdSet }, vdReport },
	vdStep,
	vdFcn,
	vdInitState
};

extern MPT_SOLVER(IVP) *mpt_vode_create()
{
	MPT_SOLVER(IVP) *sol;
	MPT_SOLVER_STRUCT(vode) *vd;
	MPT_SOLVER_IVP_STRUCT(functions) *fcn;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*vd) + sizeof(*fcn)))) {
		return 0;
	}
	vd = (void *) (sol + 1);
	mpt_vode_init(vd);
	
	fcn = MPT_IVPFCN_INIT(vd + 1);
	fcn->dae.param = vd;
	
	sol->_vptr = &vodeCtl;
	
	return sol;
}

