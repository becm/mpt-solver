/*!
 * generic solver interface for CVode.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "cvode/cvode_impl.h"
#include "sundials.h"

static void cVodeUnref(MPT_INTERFACE(object) *gen)
{
	sundials_cvode_fini((MPT_SOLVER_STRUCT(cvode) *) (gen+1));
	free(gen);
}
static uintptr_t cVodeRef(MPT_INTERFACE(object) *gen)
{
	(void) gen;
	return 0;
}
static int cVodeGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return sundials_cvode_get((MPT_SOLVER_STRUCT(cvode) *) (gen+1), pr);
}
static int cVodeSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return sundials_cvode_set((MPT_SOLVER_STRUCT(cvode) *) (gen+1), pr, src);
}

static int cVodeReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return sundials_cvode_report((MPT_SOLVER_STRUCT(cvode) *) (gen+1), what, out, data);
}

static int cVodeStep(MPT_SOLVER(IVP) *sol, double *end)
{
	MPT_SOLVER_STRUCT(cvode) *cv = (void *) (sol+1);
	int ret;
	if (!end) return sundials_cvode_prepare(cv);
	ret = sundials_cvode_step(cv, *end);
	*end = cv->t;
	return ret;
}
static void *cVodeFcn(MPT_SOLVER(IVP) *sol, int type)
{
	MPT_SOLVER_STRUCT(cvode) *cv = (void *) (sol+1);
	switch (type) {
	  case MPT_SOLVER_ENUM(ODE): return cv->ivp.pint ? 0 : (cv + 1);
	  case MPT_SOLVER_ENUM(PDE): return cv->ivp.pint ? (cv + 1) : 0;
	  default: return 0;
	}
}
static double *cVodeState(MPT_SOLVER(IVP) *sol)
{
	MPT_SOLVER_STRUCT(cvode) *cv = (void *) (sol+1);
#ifdef SUNDIALS_DOUBLE_PRECISION
	if (cv->sd.y) {
		return N_VGetArrayPointer(cv->sd.y);
	}
#endif
	return 0;
}
static const MPT_INTERFACE_VPTR(solver_ivp) cVodeCtl = {
	{ { cVodeUnref, cVodeRef, cVodeGet, cVodeSet }, cVodeReport },
	cVodeStep,
	cVodeFcn,
	cVodeState
};

extern MPT_SOLVER(IVP) *sundials_cvode_create()
{
	MPT_SOLVER(IVP) *sol;
	MPT_SOLVER_TYPE(ivpfcn) *uf;
	MPT_SOLVER_STRUCT(cvode) *cv;
	
	if (!(sol = malloc(sizeof(*sol)+sizeof(*cv)+sizeof(*uf)))) {
		return 0;
	}
	cv = (MPT_SOLVER_STRUCT(cvode) *) (sol+1);
	
	if (sundials_cvode_init(cv) < 0) {
		free(sol);
		return 0;
	}
	uf = MPT_IVPFCN_INIT(cv + 1);
	uf->ode.param = &cv->ivp;
	
	cv->ufcn = &uf->ode;
	CVodeSetUserData(cv->mem, cv);
	
	sol->_vptr = &cVodeCtl;
	
	return sol;
}
