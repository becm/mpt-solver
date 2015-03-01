/*!
 * generic solver interface for PORT
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "portn2.h"

extern int mpt_portn2_step_(MPT_SOLVER_STRUCT(portn2) *n2, double *u, double *f, const MPT_SOLVER_STRUCT(nlsfcn) *ufcn)
{
	if (!n2->iv.iov_base || ((int *) n2->iv.iov_base)[0] < 0 || !f) {
		int err;
		if (ufcn && (err = mpt_portn2_ufcn(n2, ufcn)) < 0) return err;
		if ((err = mpt_portn2_prepare(n2, n2->nls.nval, n2->nls.nres)) < 0 || !f) return err;
	}
	return mpt_portn2_step(n2, u, f);
}

static int n2Fini(MPT_INTERFACE(metatype) *gen)
{ mpt_portn2_fini((MPT_SOLVER_STRUCT(portn2) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *n2Addref()
{ return 0; }

static int n2Property(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_portn2_property((MPT_SOLVER_STRUCT(portn2) *) (gen+1), pr, src); }

static int n2Report(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_portn2_report((MPT_SOLVER_STRUCT(portn2) *) (gen+1), what, out, data); }

static void *n2Cast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int n2Step(MPT_SOLVER_INTERFACE *gen, double *x, double *f)
{
	MPT_INTERFACE_VPTR(Nls) *nctl = (void *) gen->_vptr;
	return mpt_portn2_step_((MPT_SOLVER_STRUCT(portn2) *) (gen+1), x, f, nctl->ufcn(gen));
}

static MPT_SOLVER_STRUCT(nlsfcn) *n2Fcn(const MPT_SOLVER_INTERFACE *gen)
{ return (MPT_SOLVER_STRUCT(nlsfcn) *) (((MPT_SOLVER_STRUCT(portn2 *)) (gen+1)) + 1); }

static const MPT_INTERFACE_VPTR(Nls) n2Ctl = {
	{ { n2Fini, n2Addref, n2Property, n2Cast }, n2Report },
	n2Step,
	n2Fcn
};

extern MPT_SOLVER_INTERFACE *mpt_portn2_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(portn2) *n2;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*n2) + sizeof(MPT_SOLVER_STRUCT(nlsfcn)))))
		return 0;
	
	n2 = (MPT_SOLVER_STRUCT(portn2) *) (gen+1);
	if (mpt_portn2_init(n2) < 0) { free(n2); return 0; }
	(void) MPT_NLSFCN_INIT(n2+1);
	
	gen->_vptr = &n2Ctl.gen;
	
	return gen;
}

