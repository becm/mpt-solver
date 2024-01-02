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
/* convertable interface */
static int bacConv(MPT_INTERFACE(convertable) *sol, MPT_TYPE(type) type, void *ptr)
{
	MPT_STRUCT(BacolData) *bac = (void *) sol;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&bac->_sol, &bac->_obj, type, ptr);
}
/* metatype interface */
static void bacFini(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(BacolData) *bac = (void *) mt;
	mpt_bacol_fini(&bac->d);
	mpt_bacol_output_fini(&bac->out);
	free(bac);
}
static uintptr_t bacAddref(MPT_INTERFACE(metatype) *in)
{
	(void) in;
	return 0;
}
static MPT_INTERFACE(metatype) *bacClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int bacReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(property_handler) out, void *data)
{
	MPT_STRUCT(BacolData) *bac = MPT_baseaddr(BacolData, sol, _sol);
	const MPT_SOLVER_STRUCT(bacol_out) *bac_out = 0;
	
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(PDE);
	}
	if (what & MPT_SOLVER_ENUM(Values)) {
		if (bac->out.nint || mpt_bacol_values(&bac->out, &bac->d)) {
			bac_out = &bac->out;
		}
	}
	return mpt_bacol_report(&bac->d, bac_out, what, out, data);
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
static int bacSet(MPT_INTERFACE(object) *obj, const char *pr, MPT_INTERFACE(convertable) *src)
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
		{ bacConv },
		bacFini,
		bacAddref,
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
