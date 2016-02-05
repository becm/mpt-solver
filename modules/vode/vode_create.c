/*!
 * generic solver interface for dVODE
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vode.h"

static void vdFini(MPT_INTERFACE(object) *gen)
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
static int vdSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_vode_set((MPT_SOLVER_STRUCT(vode) *) (gen+1), pr, src);
}

static int vdReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_vode_report((MPT_SOLVER_STRUCT(vode) *) (gen+1), what, out, data);
}
static int vdStep(MPT_SOLVER_INTERFACE *gen, double *tend)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen+1);
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
static void *vdFcn(const MPT_SOLVER_INTERFACE *gen, int type)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen+1);
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): return vd->ivp.pint ? 0 : (vd+1);
	  case MPT_SOLVER_ENUM(PDE): return vd->ivp.pint ? (vd+1) : 0;
	  default: return 0;
	}
}
static double *vdInitState(MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen+1);
	return vd->y;
}
static const MPT_INTERFACE_VPTR(Ivp) vodeCtl = {
	{ { vdFini, vdAddref, vdGet, vdSet }, vdReport },
	vdStep,
	vdFcn,
	vdInitState
};

extern MPT_SOLVER_INTERFACE *mpt_vode_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(vode) *vd;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*vd) + sizeof(MPT_SOLVER_STRUCT(pdefcn))))) {
		return 0;
	}
	vd = (void *) (gen + 1);
	mpt_vode_init(vd);
	
	MPT_IVPFCN_INIT(vd + 1)->ode.param = &vd->ivp;
	
	gen->_vptr = &vodeCtl.gen;
	
	return gen;
}

