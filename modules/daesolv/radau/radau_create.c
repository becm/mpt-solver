/*!
 * generic solver interface for RADAU
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "radau.h"

extern int mpt_radau_step_(MPT_SOLVER_STRUCT(radau) *rd, double *u, double *end, const MPT_SOLVER_STRUCT(ivpfcn) *uf)
{
	int err;
	
	if (rd->count.st.nfev < 0 || !end) {
		if (uf && (err = mpt_radau_ufcn(rd, uf)) < 0) {
			return err;
		}
		if ((err = mpt_radau_prepare(rd, rd->ivp.neqs, rd->ivp.pint)) < 0 || !end) {
			return err;
		}
	}
	err = u ? mpt_radau_step(rd, u, *end) : 0;
	*end = rd->ivp.last;
	return err;
}

static int rdFini(MPT_INTERFACE(metatype) *gen)
{ mpt_radau_fini((MPT_SOLVER_STRUCT(radau) *) (gen+1)); free(gen); return 0; }

static MPT_INTERFACE(metatype) *rdAddref()
{ return 0; }

static int rdProperty(MPT_INTERFACE(metatype) *gen, MPT_STRUCT(property) *pr, MPT_INTERFACE(source) *src)
{ return mpt_radau_property((MPT_SOLVER_STRUCT(radau) *) (gen+1), pr, src); }

static void *rdCast(MPT_INTERFACE(metatype) *gen, int type)
{
	switch(type) {
	  case MPT_ENUM(TypeMeta): return gen;
	  case MPT_ENUM(TypeSolver): return gen;
	  default: return 0;
	}
}

static int rdReport(const MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{ return mpt_radau_report((MPT_SOLVER_STRUCT(radau) *) (gen+1), what, out, data); }

extern int rdStep(MPT_SOLVER_INTERFACE *gen, double *u, double *end, double *x)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (gen+1);
	(void) x;
	return mpt_radau_step_(rd, u, end, (void *) (rd+1));
}

extern MPT_SOLVER_STRUCT(ivpfcn) *rdFcn(const MPT_SOLVER_INTERFACE *gen)
{ return (void *) (((MPT_SOLVER_STRUCT(radau) *) (gen+1))+1); }

static const MPT_INTERFACE_VPTR(Ivp) radauCtl = {
	{ { rdFini, rdAddref, rdProperty, rdCast }, rdReport },
	rdStep,
	rdFcn
};

extern MPT_SOLVER_INTERFACE *mpt_radau_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(radau) *data;
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*data) + sizeof(*fcn))))
		return 0;
	
	data = (MPT_SOLVER_STRUCT(radau *)) (gen+1);
	if (mpt_radau_init(data) < 0) { free(gen); return 0; }
	
	MPT_IVPFCN_INIT(data+1)->param = &data->ivp;
	
	gen->_vptr = &radauCtl.gen;
	
	return gen;
}

