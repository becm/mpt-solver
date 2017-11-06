/*!
 * generic solver interface for PORT
 */

#include <stdlib.h>
#include <string.h>

#include "portn2.h"

MPT_STRUCT(PortN2Data) {
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	MPT_SOLVER_STRUCT(portn2) d;
	MPT_NLS_STRUCT(functions) uf;
};
/* reference interface */
static void n2Unref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(PortN2Data) *n2 = (void *) ref;
	mpt_portn2_fini(&n2->d);
	free(ref);
}
static uintptr_t n2Ref()
{
	return 0;
}
/* metatype interface */
static int n2Conv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(PortN2Data) *n2 = (void *) mt;
	
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeObject), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = &n2->_obj;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &n2->_sol;
		return MPT_ENUM(TypeObject);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *n2Clone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int n2Report(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_STRUCT(PortN2Data) *n2 = (void *) sol;
	if (!what && !out && !data) {
		return mpt_portn2_get(&n2->d, 0);
	}
	return mpt_portn2_report(&n2->d, what, out, data);
}
static int n2Fcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(PortN2Data) *n2 = (void *) sol;
	return mpt_portn2_ufcn(&n2->d, &n2->uf, type, ptr);
}
static int n2Solve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(PortN2Data) *n2 = (void *) sol;
	return mpt_portn2_solve(&n2->d);
}
/* object interface */
static int n2Get(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	MPT_STRUCT(PortN2Data) *n2 = MPT_baseaddr(PortN2Data, obj, _obj);
	return mpt_portn2_get(&n2->d, pr);
}
static int n2Set(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(PortN2Data) *n2 = MPT_baseaddr(PortN2Data, obj, _obj);
	if (!pr && !src) {
		return mpt_portn2_prepare(&n2->d);
	}
	return mpt_portn2_set(&n2->d, pr, src);
}

/*!
 * \ingroup mptNlSolvMinpack
 * \brief create PortN2 solver
 * 
 * Create PortN2 solver instance with MPT interface.
 * 
 * \return PortN2 solver instance
 */
extern MPT_SOLVER(interface) *mpt_portn2_create()
{
	static const MPT_INTERFACE_VPTR(object) n2Obj = {
		n2Get, n2Set
	};
	static const MPT_INTERFACE_VPTR(solver) n2Sol = {
		{ { n2Unref, n2Ref }, n2Conv, n2Clone },
		n2Report,
		n2Fcn,
		n2Solve
	};
	MPT_STRUCT(PortN2Data) *n2;
	
	if (!(n2 = malloc(sizeof(*n2)))) {
		return 0;
	}
	if (mpt_portn2_init(&n2->d) < 0) {
		free(n2);
		return 0;
	}
	memset(&n2->uf, 0, sizeof(n2->uf));
	
	n2->_sol._vptr = &n2Sol;
	n2->_obj._vptr = &n2Obj;
	
	return &n2->_sol;
}

