/*!
 * generic solver interface for RADAU
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "radau.h"

static void rdFini(MPT_INTERFACE(object) *gen)
{
	mpt_radau_fini((MPT_SOLVER_STRUCT(radau) *) (gen+1));
	free(gen);
}
static uintptr_t rdAddref()
{
	return 0;
}
static int rdGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return mpt_radau_get((MPT_SOLVER_STRUCT(radau) *) (gen+1), pr);
}
static int rdSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_radau_set((MPT_SOLVER_STRUCT(radau) *) (gen+1), pr, src);
}

static int rdReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_radau_report((MPT_SOLVER_STRUCT(radau) *) (gen+1), what, out, data);
}

extern int rdStep(MPT_SOLVER_INTERFACE *gen, double *tend)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (gen + 1);
	int err;
	
	if (!tend) {
		if (!rd->fcn && (err = mpt_radau_ufcn(rd, (void *) (rd + 1))) < 0) {
			return err;
		}
		return mpt_radau_prepare(rd);
	}
	err = mpt_radau_step(rd, *tend);
	*tend = rd->t;
	return err;
}
static void *rdFcn(MPT_SOLVER_INTERFACE *gen, int type)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (gen + 1);
	MPT_SOLVER_TYPE(ivpfcn) *ivp = (void *) (rd + 1);
	
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return rd->ivp.pint ? 0 : &ivp->dae;
	  case MPT_SOLVER_ENUM(PDE): return rd->ivp.pint ? &ivp->pde : 0;
	  default: return 0;
	}
	if (rd->ivp.pint) {
		return 0;
	}
	ivp->dae.mas = 0;
	return &ivp->ode;
}
static double *rdState(MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (gen + 1);
	return rd->y;
}

static const MPT_INTERFACE_VPTR(Ivp) radauCtl = {
	{ { rdFini, rdAddref, rdGet, rdSet }, rdReport },
	rdStep,
	rdFcn,
	rdState
};

extern MPT_SOLVER_INTERFACE *mpt_radau_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(radau) *data;
	MPT_SOLVER_TYPE(ivpfcn) *fcn;
	
	if (!(gen = malloc(sizeof(*gen) + sizeof(*data) + sizeof(*fcn)))) {
		return 0;
	}
	data = (MPT_SOLVER_STRUCT(radau *)) (gen+1);
	mpt_radau_init(data);
	
	fcn = MPT_IVPFCN_INIT(data+1);
	fcn->ode.param = &data->ivp;
	
	gen->_vptr = &radauCtl.gen;
	
	return gen;
}

