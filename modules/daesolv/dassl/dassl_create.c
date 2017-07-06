/*!
 * generic solver interface for dDASSL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "meta.h"

#include "dassl.h"

static void ddFini(MPT_INTERFACE(unrefable) *ref)
{
	mpt_dassl_fini((MPT_SOLVER_STRUCT(dassl *)) (ref + 1));
	free(ref);
}
static uintptr_t ddAddref()
{
	return 0;
}
static int ddGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return mpt_dassl_get((void *) (obj + 1), pr);
}
static int ddSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	return _mpt_dassl_set((void *) (obj + 1), pr, src);
}

static int ddReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	return mpt_dassl_report((MPT_SOLVER_STRUCT(dassl *)) (sol + 1), what, out, data);
}
static int ddFcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	MPT_SOLVER_STRUCT(dassl) *da = (void *) (sol + 1);
	return mpt_dassl_ufcn(da, (void *) (da + 1), type, ptr);
}
static const MPT_INTERFACE_VPTR(solver) dasslCtl = {
	{ { ddFini }, ddAddref, ddGet, ddSet },
	ddReport,
	ddFcn
};


extern int _mpt_dassl_set(MPT_SOLVER_STRUCT(dassl) *da, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return mpt_dassl_prepare(da);
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		double end;
		int ret;
		
		if (!src) MPT_ERROR(BadValue);
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return mpt_dassl_step(da, end);
	}
	return mpt_dassl_set(da, pr, src);
}

extern MPT_SOLVER(generic) *mpt_dassl_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(dassl) *da;
	MPT_IVP_STRUCT(daefcn) *fcn;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*da) + sizeof(*fcn)))) {
		return 0;
	}
	da = (MPT_SOLVER_STRUCT(dassl *)) (sol+1);
	mpt_dassl_init(da);
	
	sol->_vptr = &dasslCtl;
	
	da->ipar = memset(da + 1, 0, sizeof(*fcn));
	da->rpar = (void *) da;
	
	return sol;
}
