/*!
 * generic solver interface for MINPACK
 */

#include <stdlib.h>
#include <string.h>

#include "minpack.h"

static void mpUnref(MPT_INTERFACE(unrefable) *ref)
{
	mpt_minpack_fini((MPT_SOLVER_STRUCT(minpack) *) (ref + 1));
	free(ref);
}
static uintptr_t mpRef()
{
	return 0;
}
static int mpGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return mpt_minpack_get((MPT_SOLVER_STRUCT(minpack) *) (obj + 1), pr);
}
static int mpSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_SOLVER_STRUCT(minpack) *mp = (void *) (obj + 1);
	return _mpt_minpack_set(mp, pr, src);
}

static int mpReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_minpack_report((MPT_SOLVER_STRUCT(minpack) *) (sol + 1), what, out, data);
}
static int mpFcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	MPT_SOLVER_STRUCT(minpack) *mp = (void *) (sol + 1);
	return mpt_minpack_ufcn(mp, (void *) (mp + 1), type, ptr);
}
static const MPT_INTERFACE_VPTR(solver) mpCtl = {
	{ { mpUnref }, mpRef, mpGet, mpSet },
	mpReport,
	mpFcn
};

extern int _mpt_minpack_set(MPT_SOLVER_STRUCT(minpack) *mp, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr && !src) {
		int ret;
		if (mp->info < 0 && (ret = mpt_minpack_prepare(mp))) {
			return ret;
		}
		return mpt_minpack_solve(mp);
	}
	return mpt_minpack_set(mp, pr, src);
}
extern int mpt_minpack_ufcn(MPT_SOLVER_STRUCT(minpack) *mp, MPT_SOLVER_NLS_STRUCT(functions) *fcn, int type, const void *ptr)
{
	if (mp->nls.nres != mp->nls.nval) {
		return mpt_minpack_ufcn_lmderv(mp, fcn, type, ptr);
	}
	return mpt_minpack_ufcn_hybrid(mp, fcn, type, ptr);
}

extern MPT_SOLVER(generic) *mpt_minpack_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(minpack) *mp;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*mp) + sizeof(*mp->ufcn)))) {
		return 0;
	}
	mp = (MPT_SOLVER_STRUCT(minpack) *) (sol + 1);
	mpt_minpack_init(mp);
	
	mp->ufcn = memset(mp + 1, 0, sizeof(*mp->ufcn));
	
	sol->_vptr = &mpCtl;
	
	return sol;
}

