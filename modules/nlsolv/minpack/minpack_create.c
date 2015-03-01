/*!
 * generic solver interface for MINPACK
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "minpack.h"

extern int mpt_minpack_step_(MPT_SOLVER_STRUCT(minpack) *mp, double *x, double *f)
{
	if (mp->info < 0 || !f) {
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
		if ((err = mpt_minpack_prepare(mp, mp->nls.nval, mp->nls.nres)) < 0 || !f) {
			return err;
		}
	}
	return mpt_minpack_step(mp, x, f);
}

static int mpFree(MPT_INTERFACE(metatype) *gen)
{ mpt_minpack_fini((MPT_SOLVER_STRUCT(minpack) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *mpAddref()
{ return 0; }

static int mpProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_minpack_property((MPT_SOLVER_STRUCT(minpack) *) (gen+1), pr, src); }

static void *mpCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int mpReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_minpack_report((MPT_SOLVER_STRUCT(minpack) *) (gen+1), what, out, data); }

static int mpStep(MPT_SOLVER_INTERFACE *gen, double *x, double *f)
{ return mpt_minpack_step_((MPT_SOLVER_STRUCT(minpack) *) (gen+1), x, f); }

static MPT_SOLVER_STRUCT(nlsfcn) *mpFcn(const MPT_SOLVER_INTERFACE *gen)
{ return (void *) (((MPT_SOLVER_STRUCT(minpack *)) (gen+1))->ufcn); }

static const MPT_INTERFACE_VPTR(Nls) mpCtl = {
	{ { mpFree, mpAddref, mpProperty, mpCast }, mpReport },
	mpStep,
	mpFcn
};

extern MPT_SOLVER_INTERFACE *mpt_minpack_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(minpack) *mp;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*mp) + sizeof(MPT_SOLVER_STRUCT(nlsfcn))))) {
		return 0;
	}
	mp = (MPT_SOLVER_STRUCT(minpack) *) (gen+1);
	mpt_minpack_init(mp);
	
	mp->ufcn = MPT_NLSFCN_INIT(mp + 1);
	
	gen->_vptr = &mpCtl.gen;
	
	return gen;
}

