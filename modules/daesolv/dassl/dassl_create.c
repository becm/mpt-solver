/*!
 * generic solver interface for dDASSL
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "module.h"

#include "dassl.h"

MPT_STRUCT(DasslData) {
	MPT_STRUCT(module_generic) _gen;
	
	MPT_SOLVER_STRUCT(dassl) d;
	MPT_IVP_STRUCT(daefcn)   uf;
	
	double next;
};
/* reference interface */
static void ddFini(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(DasslData) *da = (void *) ref;
	mpt_dassl_fini(&da->d);
	free(ref);
}
static uintptr_t ddAddref()
{
	return 0;
}
/* metatype interface */
static int ddConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(DasslData) *da = (void *) mt;
	return mpt_module_generic_conv(&da->_gen, type, ptr);
}
static MPT_INTERFACE(metatype) *ddClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int ddReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	const MPT_STRUCT(DasslData) *da = (void *) sol;
	if (!what && !out && !data) {
		return mpt_dassl_get(&da->d, 0);
	}
	return mpt_dassl_report(&da->d, what, out, data);
}
static int ddFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(DasslData) *da = (void *) sol;
	return mpt_dassl_ufcn(&da->d, &da->uf, type, ptr);
}
static int ddSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(DasslData) *da = (void *) sol;
	return mpt_dassl_step(&da->d, da->next);
}
/* object interface */
static int ddGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, obj, _gen._obj);
	return mpt_dassl_get(&da->d, pr);
}
static int ddSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(DasslData) *da = MPT_baseaddr(DasslData, obj, _gen._obj);
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
extern MPT_SOLVER(interface) *mpt_dassl_create()
{
	static const MPT_INTERFACE_VPTR(object) dasslObj = {
		ddGet, ddSet
	};
	static const MPT_INTERFACE_VPTR(solver) dasslSol = {
		{ { ddFini, ddAddref }, ddConv, ddClone },
		ddReport,
		ddFcn,
		ddSolve
	};
	MPT_STRUCT(DasslData) *da;
	
	if (!(da = malloc(sizeof(*da)))) {
		return 0;
	}
	mpt_dassl_init(&da->d);
	da->d.ipar = memset(&da->uf, 0, sizeof(da->uf));
	da->d.rpar = (void *) &da->d;
	
	da->_gen._mt._vptr  = &dasslSol.meta;
	da->_gen._obj._vptr = &dasslObj;
	
	return (void *) &da->_gen._mt;
}
