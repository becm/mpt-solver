/*!
 * generic solver interface for IDA.
 */

#include <stdlib.h>
#include <string.h>

#include <ida/ida.h>

#include "meta.h"

#include "sundials.h"

#include "module_functions.h"

static void idaUnref(MPT_INTERFACE(unrefable) *ref)
{
	  sundials_ida_fini((MPT_SOLVER_STRUCT(ida) *) (ref + 1));
	  free(ref);
}
static uintptr_t idaRef()
{
	return 0;
}
static int idaGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return sundials_ida_get((MPT_SOLVER_STRUCT(ida) *) (obj + 1), pr);
}
static int idaSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	return _sundials_ida_set((MPT_SOLVER_STRUCT(ida) *) (obj + 1), pr, src);
}

static int idaReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	return sundials_ida_report((MPT_SOLVER_STRUCT(ida) *) (sol + 1), what, out, data);
}
static int idaFcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	MPT_SOLVER_STRUCT(ida) *ida = (void *) (sol + 1);
	MPT_IVP_STRUCT(daefcn) *uf = (void *) (ida + 1);
	int ret;
	
	if ((ret = MPT_SOLVER_MODULE_FCN(ufcn_dae)(ida->ivp.pint, uf, type, ptr)) < 0) {
		return ret;
	}
	ida->ufcn = uf;
	return ret;
}

static const MPT_INTERFACE_VPTR(solver) idaCtl = {
	{ { idaUnref }, idaRef, idaGet, idaSet },
	idaReport,
	idaFcn
};

extern int _sundials_ida_set(MPT_SOLVER_STRUCT(ida) *ida, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return sundials_ida_prepare(ida);
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		double end;
		int ret;
		
		if (!src) return MPT_ERROR(BadValue);
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return sundials_ida_step(ida, end);
	}
	return sundials_ida_set(ida, pr, src);
}

/*!
 * \ingroup mptSundialsIda
 * \brief create IDA solver
 * 
 * Create solver interface to Sundials IDA solver.
 * 
 * \return IDA solver instance
 */
extern MPT_SOLVER(generic) *sundials_ida_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(ida) *ida;
	MPT_IVP_STRUCT(daefcn) *uf;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*ida) + sizeof(*uf)))) {
		return 0;
	}
	ida = (void *) (sol + 1);
	
	if (sundials_ida_init(ida) < 0) {
		free(ida);
		return 0;
	}
	uf = memset(ida + 1, 0, sizeof(*uf));
	
	ida->ufcn = uf;
	IDASetUserData(ida->mem, ida);
	
	sol->_vptr = &idaCtl;
	
	return sol;
}
