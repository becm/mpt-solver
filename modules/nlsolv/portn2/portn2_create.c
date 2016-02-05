/*!
 * generic solver interface for PORT
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "portn2.h"

struct _mpt_portn2_data {
	MPT_SOLVER_INTERFACE      sol;
	MPT_SOLVER_STRUCT(portn2) n2;
	MPT_SOLVER_STRUCT(nlsfcn) uf;
	double                   *res;
};

static void n2Unref(MPT_INTERFACE(object) *gen)
{
	struct _mpt_portn2_data *d = (void *) gen;
	mpt_portn2_fini(&d->n2);
	free(gen);
}
static uintptr_t n2Ref()
{
	return 0;
}
static int n2Get(const MPT_INTERFACE(object) *gen, MPT_STRUCT(property) *pr)
{
	return mpt_portn2_get((MPT_SOLVER_STRUCT(portn2) *) (gen+1), pr);
  
}
static int n2Set(MPT_INTERFACE(object) *gen, const char *pr, MPT_INTERFACE(metatype) *src)
{
	return mpt_portn2_set((MPT_SOLVER_STRUCT(portn2) *) (gen+1), pr, src);
}

static int n2Report(MPT_SOLVER_INTERFACE *gen, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	return mpt_portn2_report((MPT_SOLVER_STRUCT(portn2) *) (gen+1), what, out, data);
}
static int n2Solve(MPT_SOLVER_INTERFACE *gen)
{
	struct _mpt_portn2_data *d = (void *) gen;
	int ret;
	
	if (d->uf.res && (ret = mpt_portn2_ufcn(&d->n2, &d->uf)) < 0) {
		return ret;
	}
	if (mpt_portn2_prepare(&d->n2) < 0) {
		return ret;
	}
	return mpt_portn2_solve(&d->n2);
}

static MPT_SOLVER_STRUCT(nlsfcn) *n2Fcn(MPT_SOLVER_INTERFACE *gen)
{
	struct _mpt_portn2_data *d = (void *) gen;
	return &d->uf;
}
static const MPT_INTERFACE_VPTR(Nls) n2Ctl = {
	{ { n2Unref, n2Ref, n2Get, n2Set }, n2Report },
	n2Solve,
	n2Fcn
};

extern MPT_SOLVER_INTERFACE *mpt_portn2_create()
{
	struct _mpt_portn2_data *d;
	
	
	if (!(d = malloc(sizeof(*d)))) {
		return 0;
	}
	if (mpt_portn2_init(&d->n2) < 0) {
		free(d);
		return 0;
	}
	(void) MPT_NLSFCN_INIT(&d->n2);
	
	d->sol._vptr = &n2Ctl.gen;
	
	return &d->sol;
}

