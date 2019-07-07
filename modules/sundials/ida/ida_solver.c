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
/* convertable interface */
static int idaConv(MPT_INTERFACE(convertable) *sol, int type, void *ptr)
{
	const MPT_STRUCT(SundialsIDA) *ida = (void *) sol;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&ida->_sol, &ida->_obj, type, ptr);
}
/* metatype interface */
static void idaUnref(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(SundialsIDA) *ida = (void *) mt;
	mpt_sundials_ida_fini(&ida->d);
	free(ida);
}
static uintptr_t idaRef()
{
	return 0;
}
static MPT_INTERFACE(metatype) *idaClone()
{
	return 0;
}
/* solver interface */
static int idaReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(property_handler) out, void *data)
{
	const MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, sol, _sol);
	if (!what && !out && !data) {
		return mpt_sundials_ida_get(&ida->d, 0);
	}
	return mpt_sundials_ida_report(&ida->d, what, out, data);
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
	return mpt_sundials_ida_step(&ida->d, ida->next);
}
/* object interface */
static int idaGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, obj, _obj);
	return mpt_sundials_ida_get(&ida->d, pr);
}
static int idaSet(MPT_INTERFACE(object) *obj, const char *pr, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(SundialsIDA) *ida = MPT_baseaddr(SundialsIDA, obj, _obj);
	
	if (!pr) {
		if (!src) {
			int ret = mpt_sundials_ida_prepare(&ida->d);
			if (ret >= 0) {
				ida->next = ida->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		return mpt_solver_module_nextval(&ida->next, ida->d.t, src);
	}
	return mpt_sundials_ida_set(&ida->d, pr, src);
}

/*!
 * \ingroup mptSundialsIda
 * \brief create IDA solver
 * 
 * Create Sundials IDA solver instance with MPT interface.
 * 
 * \return IDA solver instance
 */
extern MPT_INTERFACE(metatype) *mpt_sundials_ida()
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
		{ idaConv },
		idaUnref,
		idaRef,
		idaClone
	};
	MPT_STRUCT(SundialsIDA) *ida;
	
	if (!(ida = malloc(sizeof(*ida)))) {
		return 0;
	}
	if (mpt_sundials_ida_init(&ida->d) < 0) {
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
