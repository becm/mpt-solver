/*!
 * generic solver interface for MEBDFI
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mebdfi.h"

extern int mpt_mebdfi_step_(MPT_SOLVER_STRUCT(mebdfi) *md, double *u, double *end, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	int err;
	if (md->state < 0 || !end) {
		if (ufcn && (err = mpt_mebdfi_ufcn(md, ufcn)) < 0) {
			return err;
		}
		if ((err = mpt_mebdfi_prepare(md, md->ivp.neqs, md->ivp.pint)) < 0 || !end) {
			return err;
		}
	}
	err = u ? mpt_mebdfi_step(md, u, *end) : 0;
	*end = md->ivp.last;
	return err;
}

static int meFini(MPT_INTERFACE(metatype) *gen)
{ mpt_mebdfi_fini((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *meAddref()
{ return 0; }

static int meProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_mebdfi_property((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1), pr, src); }

static void *meCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int meReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_mebdfi_report((MPT_SOLVER_STRUCT(mebdfi *)) (gen+1), what, out, data); }

extern int meStep(MPT_SOLVER_INTERFACE *gen, double *u, double *end, double *x)
{
	MPT_SOLVER_STRUCT(mebdfi) *md = (void *) (gen+1);
	(void) x;
	return mpt_mebdfi_step_(md, u, end, (void *) (md+1));
}

extern MPT_SOLVER_STRUCT(ivpfcn) *meFcn(const MPT_SOLVER_INTERFACE *gen)
{ return (void *) (((MPT_SOLVER_STRUCT(mebdfi) *) (gen+1)) + 1); }

static const MPT_INTERFACE_VPTR(Ivp) mebdfiCtl = {
	{ { meFini, meAddref, meProperty, meCast }, meReport },
	meStep,
	meFcn
};

extern MPT_SOLVER_INTERFACE *mpt_mebdfi_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(mebdfi) *md;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*md) + sizeof(MPT_SOLVER_STRUCT(ivpfcn))))) {
		return 0;
	}
	md = (MPT_SOLVER_STRUCT(mebdfi) *) (gen+1);
	if (mpt_mebdfi_init(md) < 0) {
		free(md);
		return 0;
	}
	MPT_IVPFCN_INIT(md + 1)->param = &md->ivp;
	
	gen->_vptr = &mebdfiCtl.gen;
	
	return gen;
}

