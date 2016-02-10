/*!
 * generic solver interface for MINPACK
 */

#include <stdlib.h>
#include <string.h>

#include "minpack.h"

static void mpUnref(MPT_INTERFACE(object) *obj)
{
	mpt_minpack_fini((MPT_SOLVER_STRUCT(minpack) *) (obj+1));
	free(obj);
}
static uintptr_t mpRef()
{
	return 0;
}
static int mpGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return mpt_minpack_get((MPT_SOLVER_STRUCT(minpack) *) (obj+1), pr);
}
static int mpSet(MPT_INTERFACE(object) *obj, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_minpack_set((MPT_SOLVER_STRUCT(minpack) *) (obj+1), pr, src);
}

static int mpReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_minpack_report((MPT_SOLVER_STRUCT(minpack) *) (gen+1), what, out, data);
}

static int mpSolve(MPT_SOLVER(NLS) *sol)
{
	MPT_SOLVER_STRUCT(minpack) *mp = (void *) (sol+1);
	int err;
	
	if (mp->ufcn) {
		if (mp->nls.nres != mp->nls.nval) {
			if ((err = mpt_minpack_ufcn_lmderv(mp)) < 0) {
				return err;
			}
		}
		else if ((err = mpt_minpack_ufcn_hybrid(mp)) < 0) {
			return err;
		}
	}
	if (mp->info < 0  && (err = mpt_minpack_prepare(mp))) {
		return err;
	}
	return mpt_minpack_solve(mp);
}

static MPT_SOLVER_NLS_STRUCT(functions) *mpFcn(MPT_SOLVER(NLS) *sol)
{
	MPT_SOLVER_STRUCT(minpack) *mp = (void *) (sol+1);
	return (void *) (mp+1);
}
static const MPT_INTERFACE_VPTR(solver_nls) mpCtl = {
	{ { mpUnref, mpRef, mpGet, mpSet }, mpReport },
	mpSolve,
	mpFcn
};

extern MPT_SOLVER(NLS) *mpt_minpack_create()
{
	MPT_SOLVER(NLS) *sol;
	MPT_SOLVER_STRUCT(minpack) *mp;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*mp) + sizeof(MPT_SOLVER_NLS_STRUCT(functions))))) {
		return 0;
	}
	mp = (MPT_SOLVER_STRUCT(minpack) *) (sol+1);
	mpt_minpack_init(mp);
	
	mp->ufcn = MPT_NLSFCN_INIT(mp + 1);
	
	sol->_vptr = &mpCtl;
	
	return sol;
}

