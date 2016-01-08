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

static int bacReport(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_bacol_report((MPT_SOLVER_STRUCT(bacol) *) (gen+1), what, out, data);
}
static int bacStep(MPT_SOLVER_INTERFACE *gen, double *end)
{
	MPT_SOLVER_STRUCT(bacol) *bac = (void *) (gen+1);
	int ret;
	if (!end) return mpt_bacol_prepare(bac);
	ret = mpt_bacol_step(bac, *end);
	*end = bac->t;
	return ret;
}
static void *bacFcn(const MPT_SOLVER_INTERFACE *gen, int type)
{
	(void) gen;
	(void) type;
	return 0;
}
static double *bacState(MPT_SOLVER_INTERFACE *gen)
{
	MPT_SOLVER_STRUCT(bacol) *bac = (void *) (gen+1);
	return bac->out.y;
}
static const MPT_INTERFACE_VPTR(Ivp) _vptr_bacol = {
	{ { bacFini, bacAddref, bacGet, bacSet }, bacReport },
	bacStep,
	bacFcn,
	bacState
};

extern MPT_SOLVER_INTERFACE *mpt_bacol_create()
{
	MPT_SOLVER_INTERFACE *gen;
	MPT_SOLVER_STRUCT(bacol) *data;
	
	if (!(gen = malloc(sizeof(*gen) * sizeof(*data))))
		return 0;
	
	data = (MPT_SOLVER_STRUCT(bacol) *) (gen+1);
	mpt_bacol_init(data);
	gen->_vptr = &_vptr_bacol.gen;
	
	return gen;
}

