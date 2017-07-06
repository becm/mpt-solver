/*!
 * generic solver interface for LIMEX
 */

#include <stdlib.h>
#include <string.h>

#include "meta.h"

#include "limex.h"

static MPT_SOLVER_STRUCT(limex) lxGlob;
static MPT_IVP_STRUCT(daefcn) lxGlobFcn;
static int lxReserved = 0;

extern MPT_SOLVER_STRUCT(limex) *mpt_limex_global()
{
	if (lxReserved) return &lxGlob;
	
	mpt_limex_init(&lxGlob);
	lxGlob.ufcn = memset(&lxGlobFcn, 0, sizeof(lxGlobFcn));
	lxReserved = 1;
	return &lxGlob;
}

static void lxFini(MPT_INTERFACE(unrefable) *ref)
{
	(void) ref;
	mpt_limex_fini(&lxGlob);
}
static uintptr_t lxAddref()
{
	return 0;
}
static int lxGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	(void) obj;
	return mpt_limex_get(&lxGlob, pr);
}
static int lxSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	(void) obj;
	return _mpt_limex_set(&lxGlob, pr, src);
}

static int lxReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	(void) sol;
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	return mpt_limex_report(&lxGlob, what, out, data);
}
static int lxFcn(MPT_SOLVER(generic) *sol, int type, const void *par)
{
	(void) sol;
	return mpt_limex_ufcn(&lxGlob, &lxGlobFcn, type, par);
}

extern int _mpt_limex_set(MPT_SOLVER_STRUCT(limex) *lx, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return mpt_limex_prepare(lx);
		}
	} else if (pr[0] == 't' && !pr[1]) {
		double end;
		int ret;
		
		if (!src) return MPT_ERROR(BadValue);
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return mpt_limex_step(lx, end);
	}
	return mpt_limex_set(lx, pr, src);
}

static const MPT_INTERFACE_VPTR(solver) limexCtl = {
	{ { lxFini }, lxAddref, lxGet, lxSet },
	lxReport,
	lxFcn
};

extern MPT_SOLVER(generic) *mpt_limex_create()
{
	static const MPT_SOLVER(generic) lxGlobControl = { &limexCtl };
	if (lxReserved) return 0;
	mpt_limex_global();
	return (MPT_SOLVER(generic) *) &lxGlobControl;
}
