/*!
 * generic solver interface for dVODE
 */

#include <stdlib.h>
#include <string.h>

#include "meta.h"

#include "vode.h"

static void vdFini(MPT_INTERFACE(unrefable) *gen)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen + 1);
	mpt_vode_fini(vd);
	free(gen);
}
static uintptr_t vdAddref()
{
	return 0;
}
static int vdGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen + 1);
	return mpt_vode_get(vd, pr);
}
static int vdSet(MPT_INTERFACE(object) *gen, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen + 1);
	return _mpt_vode_set(vd, pr, src);
}

static int vdReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen + 1);
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
	}
	return mpt_vode_report(vd, what, out, data);
}
static int vdFcn(MPT_SOLVER(generic) *gen, int type, const void *ptr)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen + 1);
	return mpt_vode_ufcn(vd, (void *) (vd + 1), type, ptr);
}
static const MPT_INTERFACE_VPTR(solver) vodeCtl = {
	{ { vdFini }, vdAddref, vdGet, vdSet },
	vdReport,
	vdFcn
};

extern int _mpt_vode_set(MPT_SOLVER_STRUCT(vode) *vd, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return mpt_vode_prepare(vd);
		}
	} else if (src && pr[0] == 't' && pr[1] == 0) {
		double end;
		int ret = src->_vptr->conv(src, 'd', &end);
		
		if (ret < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return mpt_vode_step(vd, end);
	}
	return mpt_vode_set(vd, pr, src);
}
extern MPT_SOLVER(generic) *mpt_vode_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(vode) *vd;
	MPT_IVP_STRUCT(odefcn) *fcn;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*vd) + sizeof(*fcn)))) {
		return 0;
	}
	vd = (void *) (sol + 1);
	mpt_vode_init(vd);
	
	memset(vd + 1, 0, sizeof(*fcn));
	
	sol->_vptr = &vodeCtl;
	
	return sol;
}

