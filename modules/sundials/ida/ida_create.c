/*!
 * generic solver interface for IDA.
 */

#include <stdlib.h>
#include <string.h>

#include <ida/ida.h>

#include "meta.h"

#include "sundials.h"

#include "module_functions.h"


MPT_STRUCT(SundialsIDA) {
	MPT_SOLVER(interface)  _sol;
	MPT_INTERFACE(object)  _obj;
	MPT_SOLVER_STRUCT(ida)  d;
	MPT_IVP_STRUCT(daefcn)  uf;
	double                  next;
};
/* reference interface */
static void idaUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(SundialsIDA) *ida = (void *) ref;
	sundials_ida_fini(&ida->d);
	free(ref);
}
static uintptr_t idaRef()
{
	return 0;
}
/* metatype interface */
static int idaConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(SundialsIDA) *cv = (void *) mt;
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeObject), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeObject);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = &cv->_obj;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &cv->_sol;
		return MPT_ENUM(TypeObject);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *idaClone()
{
	return 0;
}
/* solver interface */
static int idaReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_STRUCT(SundialsIDA) *ida = (void *) sol;
	if (!what && !out && !data) {
		return sundials_ida_get(&ida->d, 0);
	}
	return sundials_ida_report(&ida->d, what, out, data);
}
static int idaFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(SundialsIDA) *ida = (void *) sol;
	int ret;
	
	if ((ret = MPT_SOLVER_MODULE_FCN(ufcn_dae)(ida->d.ivp.pint, &ida->uf, type, ptr)) < 0) {
		return ret;
	}
	ida->d.ufcn = &ida->uf;
	return ret;
}
static int idaSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(SundialsIDA) *ida = (void *) sol;
	return sundials_ida_step(&ida->d, ida->next);
}
/* object interface */
static int idaGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, obj, _obj);
	return sundials_ida_get(&ida->d, pr);
}
static int idaSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, obj, _obj);
	
	if (!pr) {
		if (!src) {
			int ret = sundials_ida_prepare(&ida->d);
			if (ret >= 0) {
				ida->next = ida->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		double end = ida->next;
		if (src && src->_vptr->conv(src, 'd', &end) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (end < ida->d.t) {
			return MPT_ERROR(BadValue);
		}
		ida->next = end;
		return 0;
	}
	return sundials_ida_set(&ida->d, pr, src);
}

/*!
 * \ingroup mptSundialsIda
 * \brief create IDA solver
 * 
 * Create Sundials IDA solver instance with MPT interface.
 * 
 * \return IDA solver instance
 */
extern MPT_SOLVER(interface) *sundials_ida_create()
{
	static const MPT_INTERFACE_VPTR(solver) idaSol = {
		{ { idaUnref, idaRef }, idaConv, idaClone },
		idaReport,
		idaFcn,
		idaSolve
	};
	static const MPT_INTERFACE_VPTR(object) idaObj = {
		idaGet, idaSet
	};
	MPT_STRUCT(SundialsIDA) *ida;
	
	if (!(ida = malloc(sizeof(*ida)))) {
		return 0;
	}
	if (sundials_ida_init(&ida->d) < 0) {
		free(ida);
		return 0;
	}
	memset(&ida->uf, 0, sizeof(ida->uf));
	IDASetUserData(ida->d.mem, &ida->d);
	ida->next = 0.0;
	
	ida->_sol._vptr = &idaSol;
	ida->_obj._vptr = &idaObj;
	
	return &ida->_sol;
}
