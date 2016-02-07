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

static int rdReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_radau_report((MPT_SOLVER_STRUCT(radau) *) (gen+1), what, out, data);
}

extern int rdStep(MPT_SOLVER(IVP) *sol, double *tend)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (sol + 1);
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
static void *rdFcn(MPT_SOLVER(IVP) *sol, int type)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (sol + 1);
	MPT_SOLVER_STRUCT(ivpfcn) *ivp = (void *) (rd + 1);
	
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): break;
	  case MPT_SOLVER_ENUM(DAE): return rd->ivp.pint ? 0 : &ivp->dae;
	  case MPT_SOLVER_ENUM(PDE): if (!rd->ivp.pint) return 0;
	  case MPT_SOLVER_ENUM(IVP): return ivp;
	  default: return 0;
	}
	ivp->dae.mas = 0;
	return &ivp->dae;
}
static double *rdState(MPT_SOLVER(IVP) *sol)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (sol + 1);
	return rd->y;
}

static const MPT_INTERFACE_VPTR(solver_ivp) radauCtl = {
	{ { rdFini, rdAddref, rdGet, rdSet }, rdReport },
	rdStep,
	rdFcn,
	rdState
};

extern MPT_SOLVER(IVP) *mpt_radau_create()
{
	MPT_SOLVER(IVP) *sol;
	MPT_SOLVER_STRUCT(radau) *rd;
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*rd) + sizeof(*fcn)))) {
		return 0;
	}
	rd = (MPT_SOLVER_STRUCT(radau *)) (sol+1);
	mpt_radau_init(rd);
	
	fcn = MPT_IVPFCN_INIT(rd+1);
	fcn->dae.param = rd;
	
	sol->_vptr = &radauCtl;
	
	return sol;
}

