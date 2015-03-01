/*!
 * generic solver interface for BACOL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "bacol.h"

extern void uinit_(const double *, double *, const int *);

extern int mpt_bacol_step_(MPT_SOLVER_STRUCT(bacol) *bac, double *u, double *tend, double *x)
{
	int i;
	
	if (!tend || bac->mflag.noinit < 0) {
		if ((i = mpt_bacol_prepare(bac, bac->ivp.neqs, bac->ivp.pint)) < 0)
			return i;
		
		if (!tend) {
			if ( !u || !x ) return i;
			
			for ( i = 0 ; i <= bac->ivp.pint ; i++, u += bac->ivp.neqs )
				uinit_(x++, u, &bac->ivp.neqs);
			return i;
		}
	}
	i = u ? mpt_bacol_step(bac, u, *tend, x) : 0;
	*tend = bac->ivp.last;
	return i;
}

static int bacFini(MPT_INTERFACE(metatype) *gen)
{ mpt_bacol_fini((MPT_SOLVER_STRUCT(bacol) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *bacAddref()
{ return 0; }

static int bacProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_bacol_property((MPT_SOLVER_STRUCT(bacol) *) (gen+1), pr, src); }

static void *bacCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int bacReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_bacol_report((MPT_SOLVER_STRUCT(bacol) *) (gen+1), what, out, data); }

static int bacStep(MPT_SOLVER_INTERFACE *gen, double *u, double *tend, double *x)
{ return mpt_bacol_step_((void *) (gen+1), u, tend, x); }

static MPT_SOLVER_STRUCT(ivpfcn) *bacFcn(const MPT_SOLVER_INTERFACE *gen)
{ (void) gen; return 0; }

static const MPT_INTERFACE_VPTR(Ivp) bacolCtl = {
	{ { bacFini, bacAddref, bacProperty, bacCast }, bacReport },
	bacStep,
	bacFcn
};

extern MPT_SOLVER_INTERFACE *mpt_bacol_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(bacol) *data;
	
	if (!(gen = malloc(sizeof(*gen) * sizeof(*data))))
		return 0;
	
	data = (MPT_SOLVER_STRUCT(bacol) *) (gen+1);
	mpt_bacol_init(data);
	gen->_vptr = &bacolCtl.gen;
	
	return gen;
}

