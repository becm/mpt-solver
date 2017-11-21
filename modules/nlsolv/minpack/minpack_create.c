/*!
 * generic solver interface for MINPACK
 */

#include <stdlib.h>
#include <string.h>

#include "module.h"

#include "minpack.h"

MPT_STRUCT(MinpackData) {
	MPT_STRUCT(module_generic) _gen;
	
	MPT_SOLVER_STRUCT(minpack) d;
	MPT_NLS_STRUCT(functions)  uf;
};
/* reference interface */
static void mpUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(MinpackData) *mp = (void *) ref;
	mpt_minpack_fini(&mp->d);
	free(ref);
}
static uintptr_t mpRef()
{
	return 0;
}
/* metatype interface */
static int mpConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(MinpackData) *mp = (void *) mt;
	return mpt_module_generic_conv(&mp->_gen, type, ptr);
}
static MPT_INTERFACE(metatype) *mpClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int mpReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_STRUCT(MinpackData) *mp = (void *) sol;
	if (!what && !out && !data) {
		return mpt_minpack_get(&mp->d, 0);
	}
	return mpt_minpack_report(&mp->d, what, out, data);
}
static int mpFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(MinpackData) *mp = (void *) sol;
	return mpt_minpack_ufcn(&mp->d, &mp->uf, type, ptr);
}
static int mpSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(MinpackData) *mp = (void *) sol;
	return mpt_minpack_solve(&mp->d);
}
/* object interface */
static int mpGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, obj, _gen._obj);
	return mpt_minpack_get(&mp->d, pr);
}
static int mpSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, obj, _gen._obj);
	if (!pr && !src) {
		return mpt_minpack_prepare(&mp->d);
	}
	return mpt_minpack_set(&mp->d, pr, src);
}

/*!
 * \ingroup mptNlSolvMinpack
 * \brief create MINPACK solver
 * 
 * Create MINPACK solver instance with MPT interface.
 * 
 * \return MINPACK solver instance
 */
extern MPT_SOLVER(interface) *mpt_minpack_create()
{
	static const MPT_INTERFACE_VPTR(object) mpObj = {
		mpGet, mpSet
	};
	static const MPT_INTERFACE_VPTR(solver) mpSol = {
		{ { mpUnref, mpRef }, mpConv, mpClone },
		mpReport,
		mpFcn,
		mpSolve
	};
	MPT_STRUCT(MinpackData) *mp;
	
	if (!(mp = malloc(sizeof(*mp)))) {
		return 0;
	}
	mpt_minpack_init(&mp->d);
	memset(&mp->uf, 0, sizeof(mp->uf));
	
	mp->_gen._mt._vptr  = &mpSol.meta;
	mp->_gen._obj._vptr = &mpObj;
	
	return (void *) &mp->_gen._mt;
}

