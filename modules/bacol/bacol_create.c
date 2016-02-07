/*!
 * generic solver interface for BACOL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "bacol.h"

extern void uinit_(const double *, double *, const int *);

static void bacFini(MPT_INTERFACE(object) *gen)
{
	mpt_bacol_fini((MPT_SOLVER_STRUCT(bacol) *) (gen+1));
	free(gen);
}
static uintptr_t bacAddref()
{
	return 0;
}
static int bacGet(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return mpt_bacol_get((MPT_SOLVER_STRUCT(bacol) *) (gen+1), pr);
}
static int bacSet(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_bacol_set((MPT_SOLVER_STRUCT(bacol) *) (gen+1), pr, src);
}

static int bacReport(MPT_SOLVER(generic) *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_bacol_report((MPT_SOLVER_STRUCT(bacol) *) (gen+1), what, out, data);
}

static int bacStep(MPT_SOLVER(IVP) *sol, double *end)
{
	MPT_SOLVER_STRUCT(bacol) *bac = (void *) (sol+1);
	int ret;
	if (!end) return mpt_bacol_prepare(bac);
	ret = mpt_bacol_step(bac, *end);
	*end = bac->t;
	return ret;
}
static void *bacFcn()
{
	return 0;
}
static double *bacState(MPT_SOLVER(IVP) *sol)
{
	MPT_SOLVER_STRUCT(bacol) *bac = (void *) (sol+1);
	return bac->out.y;
}
static const MPT_INTERFACE_VPTR(solver_ivp) _vptr_bacol = {
	{ { bacFini, bacAddref, bacGet, bacSet }, bacReport },
	bacStep,
	bacFcn,
	bacState
};

extern MPT_SOLVER(IVP) *mpt_bacol_create()
{
	MPT_SOLVER(IVP) *sol;
	MPT_SOLVER_STRUCT(bacol) *data;
	
	if (!(sol = malloc(sizeof(*sol) * sizeof(*data)))) {
		return 0;
	}
	data = (MPT_SOLVER_STRUCT(bacol) *) (sol+1);
	mpt_bacol_init(data);
	
	sol->_vptr = &_vptr_bacol;
	
	return sol;
}

