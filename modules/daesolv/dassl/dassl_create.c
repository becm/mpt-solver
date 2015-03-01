/*!
 * generic solver interface for dDASSL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "dassl.h"

extern int mpt_dassl_step_(MPT_SOLVER_STRUCT(dassl) *ds, double *u, double *end, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	int	err;
	
	if (!end || (ds->info[0] < 0)) {
		if (ufcn && (err = mpt_dassl_ufcn(ds, ufcn)) < 0) {
			return err;
		}
		if ((err = mpt_dassl_prepare(ds, ds->ivp.neqs, ds->ivp.pint)) < 0 || !end) {
			return err;
		}
	}
	err = u ? mpt_dassl_step(ds, u, *end) : 0;
	*end = ds->ivp.last;
	return err;
}

static int ddFini(MPT_INTERFACE(metatype) *gen)
{ mpt_dassl_fini((MPT_SOLVER_STRUCT(dassl *)) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *ddAddref()
{ return 0; }

static int ddProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_dassl_property((void *) (gen+1), pr, src); }

static int ddReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_dassl_report((MPT_SOLVER_STRUCT(dassl *)) (gen+1), what, out, data); }

static void *ddCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int dasslStep(MPT_SOLVER_INTERFACE *gen, double *u, double *end, double *x)
{
	MPT_SOLVER_STRUCT(dassl) *ds = (void *) (gen+1);
	(void) x;
	return mpt_dassl_step_(ds, u, end, (void *) (ds+1));
}

static MPT_SOLVER_STRUCT(ivpfcn) *ddFcn(const MPT_SOLVER_INTERFACE *gen)
{ return (void *) (((MPT_SOLVER_STRUCT(dassl *)) (gen+1)) + 1); }

static const MPT_INTERFACE_VPTR(Ivp) dasslCtl = {
	{ { ddFini, ddAddref, ddProperty, ddCast }, ddReport },
	dasslStep,
	ddFcn
};

extern MPT_SOLVER_INTERFACE *mpt_dassl_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(dassl) *ds;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*ds) + sizeof(MPT_SOLVER_STRUCT(ivpfcn))))) {
		return 0;
	}
	ds = (MPT_SOLVER_STRUCT(dassl *)) (gen+1);
	if (mpt_dassl_init(ds) < 0) { free(ds); return 0; }
	gen->_vptr = &dasslCtl.gen;
	
	MPT_IVPFCN_INIT(ds+1)->param = &ds->ivp;
	
	return gen;
}
