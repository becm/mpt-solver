/*!
 * generic solver interface for BACOL
 */

#include <stdlib.h>

#include "meta.h"

#include "bacol.h"

extern void uinit_(const double *, double *, const int *);

MPT_STRUCT(BacolData) {
	MPT_SOLVER(interface)   _sol;
	MPT_INTERFACE(object)   _obj;
	MPT_SOLVER_STRUCT(bacol) d;
	MPT_SOLVER_STRUCT(bacol_out) out;
	double next;
};
/* reference interface */
static void bacFini(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(BacolData) *bac = (void *) ref;
	mpt_bacol_fini(&bac->d);
	mpt_bacol_output_fini(&bac->out);
	free(ref);
}
static uintptr_t bacAddref(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int bacConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(BacolData) *bac = (void *) mt;
	
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeObject), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeObject);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = &bac->_obj;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &bac->_sol;
		return MPT_ENUM(TypeObject);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *bacClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
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
static int bacReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_STRUCT(BacolData) *bac = (void *) sol;
	
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(PDE);
	}
	if (what & MPT_SOLVER_ENUM(Values)) {
		if (!bac->out.nint) {
			mpt_bacol_values(&bac->out, &bac->d);
		}
		if (out) {
			struct outContext ctx;
			ctx.out = out;
			ctx.ctx = data;
			mpt_bacol_output_report(&bac->out, bac->d.t, outValues, &ctx);
		}
		what &= ~MPT_SOLVER_ENUM(Values);
	}
	return mpt_bacol_report(&bac->d, what, out, data);
}
static int bacFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	(void) sol; (void) type; (void) ptr;
	return MPT_ERROR(BadType);
}
static int bacSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(BacolData) *bac = (void *) sol;
	int ret = mpt_bacol_step(&bac->d, bac->next);
	if (ret >= 0) {
		bac->out.nint = 0;
	}
	return ret;
}
/* object interface */
static int bacGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(BacolData) *bac = MPT_baseaddr(BacolData, obj, _obj);
	return mpt_bacol_get(&bac->d, pr);
}
static int bacSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(BacolData) *bac = MPT_baseaddr(BacolData, obj, _obj);
	if (!pr) {
		if (!src) {
			bac->out.nint = 0;
			return mpt_bacol_prepare(&bac->d);
		}
	}
	else if (pr[0] == 't' && pr[1] == 0) {
		double end = bac->next;
		if (src && src->_vptr->conv(src, 'd', &end) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (end < bac->d.t) {
			return MPT_ERROR(BadValue);
		}
		bac->next = end;
		return 0;
	}
	return mpt_bacol_set(&bac->d, pr, src);
}

/*!
 * \ingroup mptBacol
 * \brief create BACOL solver
 * 
 * Create BACOL solver instance with MPT interface.
 * 
 * \return BACOL solver instance
 */
extern MPT_SOLVER(interface) *mpt_bacol_create()
{
	static const MPT_INTERFACE_VPTR(object) bacolObj = {
		bacGet, bacSet
	};
	static const MPT_INTERFACE_VPTR(solver) bacolSol = {
		{ { bacFini, bacAddref }, bacConv, bacClone },
		bacReport,
		bacFcn,
		bacSolve
	};
	MPT_STRUCT(BacolData) *bac;
	
	if (!(bac = malloc(sizeof(*bac)))) {
		return 0;
	}
	mpt_bacol_init(&bac->d);
	mpt_bacol_output_init(&bac->out);
	bac->next = 0;
	
	bac->_sol._vptr = &bacolSol;
	bac->_obj._vptr = &bacolObj;
	
	return &bac->_sol;
}
