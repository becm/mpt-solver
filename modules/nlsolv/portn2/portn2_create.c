/*!
 * generic solver interface for PORT
 */

#include <stdlib.h>
#include <string.h>

#include "portn2.h"

struct _mpt_portn2_data {
	MPT_SOLVER(generic) sol;
	MPT_SOLVER_STRUCT(portn2) n2;
	MPT_NLS_STRUCT(functions) uf;
};

static void n2Unref(MPT_INTERFACE(unrefable) *ref)
{
	struct _mpt_portn2_data *d = (void *) ref;
	mpt_portn2_fini(&d->n2);
	free(ref);
}
static uintptr_t n2Ref()
{
	return 0;
}
static int n2Get(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	struct _mpt_portn2_data *d = (void *) obj;
	return mpt_portn2_get(&d->n2, pr);
}
static int n2Set(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	struct _mpt_portn2_data *d = (void *) obj;
	return _mpt_portn2_set(&d->n2, pr, src);
}

static int n2Report(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	struct _mpt_portn2_data *d = (void *) sol;
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(NlsUser) | MPT_SOLVER_ENUM(NlsOverdet);
	}
	return mpt_portn2_report(&d->n2, what, out, data);
}
static int n2Fcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	struct _mpt_portn2_data *d = (void *) sol;
	return mpt_portn2_ufcn(&d->n2, &d->uf, type, ptr);
}

static const MPT_INTERFACE_VPTR(solver) n2Ctl = {
	{ { n2Unref }, n2Ref, n2Get, n2Set },
	n2Report,
	n2Fcn
};

extern int _mpt_portn2_set(MPT_SOLVER_STRUCT(portn2) *n2, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr && !src) {
		int ret, *state = n2->iv.iov_base;
		if ((!state || *state < 0) && (ret = mpt_portn2_prepare(n2)) < 0) {
			return ret;
		}
		return mpt_portn2_solve(n2);
	}
	return mpt_portn2_set(n2, pr, src);
}
extern MPT_SOLVER(generic) *mpt_portn2_create()
{
	struct _mpt_portn2_data *d;
	
	if (!(d = malloc(sizeof(*d)))) {
		return 0;
	}
	if (mpt_portn2_init(&d->n2) < 0) {
		free(d);
		return 0;
	}
	memset(&d->uf, 0, sizeof(d->uf));
	
	d->sol._vptr = &n2Ctl;
	
	return &d->sol;
}

