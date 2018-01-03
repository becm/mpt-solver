/*!
 * generic solver interface for IDA.
 */

#include <stdlib.h>
#include <string.h>

#include <ida/ida.h>

#include "sundials.h"

#include "meta.h"

#include "module_functions.h"

MPT_STRUCT(SundialsIDA) {
	MPT_INTERFACE(metatype) _mt;
	
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(ida) d;
	MPT_IVP_STRUCT(daefcn) uf;
	
	double next;
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
	const MPT_STRUCT(SundialsIDA) *ida = (void *) mt;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&ida->_sol, &ida->_obj, type, ptr);
}
static MPT_INTERFACE(metatype) *idaClone()
{
	return 0;
}
/* solver interface */
static int idaReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	const MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, sol, _sol);
	if (!what && !out && !data) {
		return sundials_ida_get(&ida->d, 0);
	}
	return sundials_ida_report(&ida->d, what, out, data);
}
static int idaFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, sol, _sol);
	int ret;
	
	if ((ret = mpt_solver_module_ufcn_dae(ida->d.ivp.pint, &ida->uf, type, ptr)) < 0) {
		return ret;
	}
	ida->d.ufcn = &ida->uf;
	return ret;
}
static int idaSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, sol, _sol);
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
		return mpt_solver_module_nextval(&ida->next, ida->d.t, src);
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
extern MPT_INTERFACE(metatype) *sundials_ida_create()
{
	static const MPT_INTERFACE_VPTR(object) idaObj = {
		idaGet, idaSet
	};
	static const MPT_INTERFACE_VPTR(solver) idaSol = {
		idaReport,
		idaFcn,
		idaSolve
	};
	static const MPT_INTERFACE_VPTR(metatype) idaMeta = {
		{ idaUnref, idaRef },
		idaConv,
		idaClone
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
	
	ida->_mt._vptr = &idaMeta;
	
	ida->_sol._vptr = &idaSol;
	ida->_obj._vptr = &idaObj;
	
	return &ida->_mt;
}
