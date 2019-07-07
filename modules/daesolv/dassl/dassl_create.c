/*!
 * generic solver interface for dDASSL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dassl.h"

#include "meta.h"

#include "module_functions.h"

MPT_STRUCT(DasslData) {
	MPT_INTERFACE(metatype) _mt;
	
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(dassl) d;
	MPT_IVP_STRUCT(daefcn)   uf;
	
	double next;
};
/* convertable interface */
static int ddConv(MPT_INTERFACE(convertable) *sol, int type, void *ptr)
{
	MPT_STRUCT(DasslData) *da = (void *) sol;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&da->_sol, &da->_obj, type, ptr);
}
/* metatype interface */
static void ddFini(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(DasslData) *da = (void *) mt;
	mpt_dassl_fini(&da->d);
	free(mt);
}
static uintptr_t ddAddref()
{
	return 0;
}
static MPT_INTERFACE(metatype) *ddClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int ddReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(property_handler) out, void *data)
{
	const MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, sol, _sol);
	if (!what && !out && !data) {
		return mpt_dassl_get(&da->d, 0);
	}
	return mpt_dassl_report(&da->d, what, out, data);
}
static int ddFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, sol, _sol);
	return mpt_dassl_ufcn(&da->d, &da->uf, type, ptr);
}
static int ddSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, sol, _sol);
	return mpt_dassl_step(&da->d, da->next);
}
/* object interface */
static int ddGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, obj, _obj);
	return mpt_dassl_get(&da->d, pr);
}
static int ddSet(MPT_INTERFACE(object) *obj, const char *pr, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, obj, _obj);
	if (!pr) {
		if (!src) {
			int ret = mpt_dassl_prepare(&da->d);
			if (ret >= 0) {
				da->next = da->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		return mpt_solver_module_nextval(&da->next, da->d.t, src);
	}
	return mpt_dassl_set(&da->d, pr, src);
}

/*!
 * \ingroup mptDaesolvDassl
 * \brief create DASSL solver
 * 
 * Create dDassl solver instance with MPT interface.
 * 
 * \return Dassl solver instance
 */
extern MPT_INTERFACE(metatype) *mpt_dassl_create()
{
	static const MPT_INTERFACE_VPTR(object) dasslObj = {
		ddGet, ddSet
	};
	static const MPT_INTERFACE_VPTR(solver) dasslSol = {
		ddReport,
		ddFcn,
		ddSolve
	};
	static const MPT_INTERFACE_VPTR(metatype) dasslMeta = {
		{ ddConv },
		ddFini,
		ddAddref,
		ddClone
	};
	MPT_STRUCT(DasslData) *da;
	
	if (!(da = malloc(sizeof(*da)))) {
		return 0;
	}
	mpt_dassl_init(&da->d);
	da->d.ipar = memset(&da->uf, 0, sizeof(da->uf));
	da->d.rpar = (void *) &da->d;
	
	da->_mt._vptr = &dasslMeta;
	
	da->_sol._vptr = &dasslSol;
	da->_obj._vptr = &dasslObj;
	
	return &da->_mt;
}
