/*!
 * generic solver interface for BACOL
 */

#include <stdlib.h>

#include "meta.h"

#include "bacol.h"

extern void uinit_(const double *, double *, const int *);

static void bacFini(MPT_INTERFACE(unrefable) *ref)
{
	mpt_bacol_fini((MPT_SOLVER_STRUCT(bacol) *) (ref + 1));
	free(ref);
}
static uintptr_t bacAddref()
{
	return 0;
}
static int bacGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return mpt_bacol_get((MPT_SOLVER_STRUCT(bacol) *) (obj + 1), pr);
}
static int bacSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_SOLVER_STRUCT(bacol) *bac = (void *) (obj + 1);
	MPT_SOLVER_STRUCT(bacol_out) *bo = (void *) (bac + 1);
	bo->nint = 0;
	return _mpt_bacol_set(bac, pr, src);
}

struct outContext
{
	MPT_TYPE(PropertyHandler) out;
	void *ctx;
};
static int outValues(void *ptr, MPT_STRUCT(value) val)
{
	struct outContext *ctx = ptr;
	MPT_STRUCT(property) pr;
	pr.name = 0;
	pr.desc = MPT_tr("solver state");
	pr.val = val;
	return ctx->out(ctx->ctx, &pr);
}
static int bacReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_SOLVER_STRUCT(bacol) *bac;
	
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(PDE);
	}
	bac = (void *) (sol + 1);
	if (what & MPT_SOLVER_ENUM(Values)) {
		MPT_SOLVER_STRUCT(bacol_out) *bo = (void *) (bac + 1);
		if (!bo->nint) {
			mpt_bacol_values(bo, bac);
		}
		if (out) {
			struct outContext ctx;
			ctx.out = out;
			ctx.ctx = data;
			mpt_bacol_output_report(bo, bac->t, outValues, &ctx);
		}
		what &= ~MPT_SOLVER_ENUM(Values);
	}
	return mpt_bacol_report(bac, what, out, data);
}
static int bacFcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	(void) sol; (void) type; (void) ptr;
	return MPT_ERROR(BadType);
}
static const MPT_INTERFACE_VPTR(solver) _vptr_bacol = {
	{ { bacFini }, bacAddref, bacGet, bacSet },
	bacReport,
	bacFcn
};


extern int _mpt_bacol_set(MPT_SOLVER_STRUCT(bacol) *bac, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return mpt_bacol_prepare(bac);
		}
	} else if (pr[0] == 't' && !pr[1]) {
		double end = 0;
		int ret;
		
		if (!src) return MPT_ERROR(BadValue);
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return mpt_bacol_step(bac, end);
	}
	return mpt_bacol_set(bac, pr, src);
}

extern MPT_SOLVER(generic) *mpt_bacol_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(bacol) *bac;
	MPT_SOLVER_STRUCT(bacol_out) *out;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*bac) + sizeof(*out)))) {
		return 0;
	}
	bac = (void *) (sol + 1);
	out = (void *) (bac + 1);
	
	mpt_bacol_init(bac);
	mpt_bacol_output_init(out);
	
	sol->_vptr = &_vptr_bacol;
	
	return sol;
}

