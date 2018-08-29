/*!
 * generic solver interface for BACOL
 */

#include <stdlib.h>

#include "bacol.h"

#include "meta.h"

#include "module_functions.h"

extern void uinit_(const double *, double *, const int *);

MPT_STRUCT(BacolData) {
	MPT_INTERFACE(metatype) _mt;
	
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(bacol)     d;
	MPT_SOLVER_STRUCT(bacol_out) out;
	
	double next;
};
/* reference interface */
static void bacFini(MPT_INTERFACE(instance) *in)
{
	MPT_STRUCT(BacolData) *bac = (void *) in;
	mpt_bacol_fini(&bac->d);
	mpt_bacol_output_fini(&bac->out);
	free(bac);
}
static uintptr_t bacAddref(MPT_INTERFACE(instance) *in)
{
	(void) in;
	return 0;
}
/* metatype interface */
static int bacConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(BacolData) *bac = (void *) mt;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&bac->_sol, &bac->_obj, type, ptr);
}
static MPT_INTERFACE(metatype) *bacClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
struct outContext
{
	MPT_TYPE(property_handler) out;
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
static int bacReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(property_handler) out, void *data)
{
	MPT_STRUCT(BacolData) *bac = MPT_baseaddr(BacolData, sol, _sol);
	
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(PDE);
	}
	if (what & MPT_SOLVER_ENUM(Values)) {
		if (!bac->out.nint && !mpt_bacol_values(&bac->out, &bac->d)) {
			return MPT_ERROR(BadOperation);
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
	MPT_STRUCT(BacolData) *bac = MPT_baseaddr(BacolData, sol, _sol);
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
		return mpt_solver_module_nextval(&bac->next, bac->d.t, src);
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
extern MPT_INTERFACE(metatype) *mpt_bacol_create()
{
	static const MPT_INTERFACE_VPTR(object) bacolObj = {
		bacGet, bacSet
	};
	static const MPT_INTERFACE_VPTR(solver) bacolSol = {
		bacReport,
		bacFcn,
		bacSolve
	};
	static const MPT_INTERFACE_VPTR(metatype) bacolMeta = {
		{ bacFini, bacAddref },
		bacConv,
		bacClone
	};
	MPT_STRUCT(BacolData) *bac;
	
	if (!(bac = malloc(sizeof(*bac)))) {
		return 0;
	}
	mpt_bacol_init(&bac->d);
	mpt_bacol_output_init(&bac->out);
	bac->next = 0;
	
	bac->_mt._vptr = &bacolMeta;
	
	bac->_sol._vptr = &bacolSol;
	bac->_obj._vptr = &bacolObj;
	
	return &bac->_mt;
}
