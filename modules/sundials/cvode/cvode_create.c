/*!
 * generic solver interface for CVode.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "meta.h"

#include "cvode/cvode_impl.h"
#include "sundials.h"

#include "module_functions.h"

static void cVodeUnref(MPT_INTERFACE(unrefable) *ref)
{
	sundials_cvode_fini((MPT_SOLVER_STRUCT(cvode) *) (ref + 1));
	free(ref);
}
static uintptr_t cVodeRef(MPT_INTERFACE(object) *obj)
{
	(void) obj;
	return 0;
}
static int cVodeGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return sundials_cvode_get((MPT_SOLVER_STRUCT(cvode) *) (obj + 1), pr);
}
static int cVodeSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	return _sundials_cvode_set((MPT_SOLVER_STRUCT(cvode) *) (obj + 1), pr, src);
}

static int cVodeReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(ODE) | MPT_SOLVER_ENUM(PDE);
	}
	return sundials_cvode_report((MPT_SOLVER_STRUCT(cvode) *) (sol + 1), what, out, data);
}
static int cVodeFcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	MPT_SOLVER_STRUCT(cvode) *cv = (void *) (sol + 1);
	MPT_IVP_STRUCT(odefcn) *uf = (void *) (cv + 1);
	int ret;
	
	if ((ret = MPT_SOLVER_MODULE_FCN(ufcn_ode)(cv->ivp.pint, uf, type, ptr)) < 0) {
		return ret;
	}
	cv->ufcn = uf;
	return ret;
}
static const MPT_INTERFACE_VPTR(solver) cVodeCtl = {
	{ { cVodeUnref }, cVodeRef, cVodeGet, cVodeSet },
	cVodeReport,
	cVodeFcn
};

extern int _sundials_cvode_set(MPT_SOLVER_STRUCT(cvode) *cv, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return sundials_cvode_prepare(cv);
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		double end;
		int ret;
		
		if (!src) return MPT_ERROR(BadValue);
		if ((ret = src->_vptr->conv(src, 'd', &end)) < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return sundials_cvode_step(cv, end);
	}
	return sundials_cvode_set(cv, pr, src);
}

extern MPT_SOLVER(generic) *sundials_cvode_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(cvode) *cv;
	MPT_IVP_STRUCT(odefcn) *uf;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*cv) + sizeof(*uf)))) {
		return 0;
	}
	cv = (MPT_SOLVER_STRUCT(cvode) *) (sol + 1);
	
	if (sundials_cvode_init(cv) < 0) {
		free(sol);
		return 0;
	}
	uf = memset(cv + 1, 0, sizeof(*cv->ufcn));
	
	cv->ufcn = uf;
	CVodeSetUserData(cv->mem, cv);
	
	sol->_vptr = &cVodeCtl;
	
	return sol;
}
