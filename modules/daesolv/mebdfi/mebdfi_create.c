/*!
 * generic solver interface for MEBDFI
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "meta.h"

#include "mebdfi.h"

static void meFini(MPT_INTERFACE(unrefable) *ref)
{
	mpt_mebdfi_fini((MPT_SOLVER_STRUCT(mebdfi *)) (ref + 1));
	free(ref);
}
static uintptr_t meAddref()
{
	return 0;
}
static int meGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return mpt_mebdfi_get((MPT_SOLVER_STRUCT(mebdfi *)) (obj + 1), pr);
}
static int meSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	return _mpt_mebdfi_set((MPT_SOLVER_STRUCT(mebdfi *)) (obj + 1), pr, src);
}

static int meReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	return mpt_mebdfi_report((MPT_SOLVER_STRUCT(mebdfi *)) (sol + 1), what, out, data);
}
static int meFcn(MPT_SOLVER(generic) *sol, int type, const void *par)
{
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) (sol + 1);
	return mpt_mebdfi_ufcn(me, (void *) (me + 1), type, par);
}

static const MPT_INTERFACE_VPTR(solver) mebdfiCtl = {
	{ { meFini }, meAddref, meGet, meSet },
	meReport,
	meFcn
};

extern int _mpt_mebdfi_set(MPT_SOLVER_STRUCT(mebdfi) *me, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return mpt_mebdfi_prepare(me);
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		double end;
		int ret;
		
		if (!src) return MPT_ERROR(BadValue);
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return mpt_mebdfi_step(me, end);
	}
	return mpt_mebdfi_set(me, pr, src);
}

extern MPT_SOLVER(generic) *mpt_mebdfi_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(mebdfi) *md;
	MPT_IVP_STRUCT(daefcn) *fcn;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*md) + sizeof(*fcn)))) {
		return 0;
	}
	md = (MPT_SOLVER_STRUCT(mebdfi) *) (sol + 1);
	mpt_mebdfi_init(md);
	
	md->ipar = memset(md + 1, 0, sizeof(*fcn));
	
	sol->_vptr = &mebdfiCtl;
	
	return sol;
}

