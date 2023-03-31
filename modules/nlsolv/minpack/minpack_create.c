/*!
 * generic solver interface for MINPACK
 */

#include <stdlib.h>
#include <string.h>

#include "minpack.h"

#include "meta.h"

#include "module_functions.h"

MPT_STRUCT(MinpackData) {
	MPT_INTERFACE(metatype) _mt;
	
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(minpack) d;
	MPT_NLS_STRUCT(functions)  uf;
};
/* convertable interface */
static int mpConv(MPT_INTERFACE(convertable) *sol, MPT_TYPE(value) type, void *ptr)
{
	const MPT_STRUCT(MinpackData) *mp = (void *) sol;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&mp->_sol, &mp->_obj, type, ptr);
}
/* metatype interface */
static void mpUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(MinpackData) *mp = (void *) mt;
	mpt_minpack_fini(&mp->d);
	free(mp);
}
static uintptr_t mpRef()
{
	return 0;
}
static MPT_INTERFACE(metatype) *mpClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int mpReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(property_handler) out, void *data)
{
	MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, sol, _sol);
	if (!what && !out && !data) {
		return mpt_minpack_get(&mp->d, 0);
	}
	return mpt_minpack_report(&mp->d, what, out, data);
}
static int mpFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, sol, _sol);
	return mpt_minpack_ufcn(&mp->d, &mp->uf, type, ptr);
}
static int mpSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, sol, _sol);
	return mpt_minpack_solve(&mp->d);
}
/* object interface */
static int mpGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, obj, _obj);
	return mpt_minpack_get(&mp->d, pr);
}
static int mpSet(MPT_INTERFACE(object) *obj, const char *pr, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(MinpackData) *mp = MPT_baseaddr(MinpackData, obj, _obj);
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
extern MPT_INTERFACE(metatype) *mpt_minpack_create()
{
	static const MPT_INTERFACE_VPTR(object) mpObj = {
		mpGet, mpSet
	};
	static const MPT_INTERFACE_VPTR(solver) mpSol = {
		mpReport,
		mpFcn,
		mpSolve
	};
	static const MPT_INTERFACE_VPTR(metatype) mpMeta = {
		{ mpConv },
		mpUnref,
		mpRef,
		mpClone
	};
	MPT_STRUCT(MinpackData) *mp;
	
	if (!(mp = malloc(sizeof(*mp)))) {
		return 0;
	}
	mpt_minpack_init(&mp->d);
	memset(&mp->uf, 0, sizeof(mp->uf));
	
	mp->_mt._vptr = &mpMeta;
	
	mp->_sol._vptr = &mpSol;
	mp->_obj._vptr = &mpObj;
	
	return &mp->_mt;
}

