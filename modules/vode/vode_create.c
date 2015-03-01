/*!
 * generic solver interface for dVODE
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vode.h"

extern int mpt_vode_step_(MPT_SOLVER_STRUCT(vode) *vd, double *u, double *tend, MPT_SOLVER_STRUCT(ivpfcn) *uf)
{
	int err;
	
	if (!tend || vd->istate < 0) {
		if (uf && (err = mpt_vode_ufcn(vd, uf)) < 0)
			return err;
		if ((err = mpt_vode_prepare(vd, vd->ivp.neqs, vd->ivp.pint)) < 0 || !tend)
			return err;
	}
	err = u ? mpt_vode_step(vd, u, *tend) : 0;
	*tend = vd->ivp.last;
	return err;
}

static int vdFini(MPT_INTERFACE(metatype) *gen)
{ mpt_vode_fini((MPT_SOLVER_STRUCT(vode) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *vdAddref()
{ return 0; }

static int vdProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_vode_property((MPT_SOLVER_STRUCT(vode) *) (gen+1), pr, src); }

static void *vdCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int vdReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_vode_report((MPT_SOLVER_STRUCT(vode) *) (gen+1), what, out, data); }

static int vdStep(MPT_SOLVER_INTERFACE *gen, double *u, double *tend, double *x)
{
	MPT_SOLVER_STRUCT(vode) *vd = (void *) (gen+1);
	(void) x;
	return mpt_vode_step_(vd, u, tend, (void *) (vd+1));
}

extern MPT_SOLVER_STRUCT(ivpfcn) *vdFcn(const MPT_SOLVER_INTERFACE *gen)
{ return (void *) (((MPT_SOLVER_STRUCT(vode *)) (gen+1)) + 1); }

static const MPT_INTERFACE_VPTR(Ivp) vodeCtl = {
	{ { vdFini, vdAddref, vdProperty, vdCast }, vdReport },
	vdStep,
	vdFcn
};

extern MPT_SOLVER_INTERFACE *mpt_vode_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(vode) *vd;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*vd) + sizeof(MPT_SOLVER_STRUCT(ivpfcn)))))
		return 0;
	
	vd = (void *) (gen + 1);
	if (mpt_vode_init(vd) < 0) { free(vd); return 0; }
	
	MPT_IVPFCN_INIT(vd + 1)->param = &vd->ivp;
	
	gen->_vptr = &vodeCtl.gen;
	
	return gen;
}

