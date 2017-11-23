/*!
 * generic solver interface for CVode.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cvode/cvode_impl.h"
#include "sundials.h"

#include "module_functions.h"

MPT_STRUCT(SundialsCVode) {
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(cvode) d;
	MPT_IVP_STRUCT(odefcn)   uf;
	
	double next;
};
/* reference interface */
static void cVodeUnref(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(SundialsCVode) *cv = (void *) ref;
	sundials_cvode_fini(&cv->d);
	free(cv);
}
static uintptr_t cVodeRef(MPT_INTERFACE(reference) *ref)
{
	(void) ref;
	return 0;
}
/* metatype interface */
static int cVodeConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(SundialsCVode) *cv = (void *) mt;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&cv->_sol, &cv->_obj, type, ptr);
}
static MPT_INTERFACE(metatype) *cVodeClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int cVodeReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	MPT_STRUCT(SundialsCVode) *cv = (void *) sol;
	if (!what && !out && !data) {
		return sundials_cvode_get(&cv->d, 0);
	}
	return sundials_cvode_report(&cv->d, what, out, data);
}
static int cVodeFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(SundialsCVode) *cv = (void *) sol;
	int ret;
	
	if ((ret = mpt_solver_module_ufcn_ode(cv->d.ivp.pint, &cv->uf, type, ptr)) < 0) {
		return ret;
	}
	cv->d.ufcn = &cv->uf;
	return ret;
}
static int cVodeSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(SundialsCVode) *cv = (void *) sol;
	return sundials_cvode_step(&cv->d, cv->next);
}
/* object interface */
static int cVodeGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(SundialsCVode) *cv = MPT_baseaddr(SundialsCVode, obj, _obj);
	return sundials_cvode_get(&cv->d, pr);
}
static int cVodeSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(SundialsCVode) *cv = MPT_baseaddr(SundialsCVode, obj, _obj);
	
	if (!pr) {
		if (!src) {
			int ret = sundials_cvode_prepare(&cv->d);
			if (ret >= 0) {
				cv->next = cv->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		return mpt_solver_module_nextval(&cv->next, cv->d.t, src);
	}
	return sundials_cvode_set(&cv->d, pr, src);
}

/*!
 * \ingroup mptSundialsCvode
 * \brief create CVode solver
 * 
 * Create Sundials CVode solver instance with MPT interface.
 * 
 * \return CVode solver instance
 */
extern MPT_SOLVER(interface) *sundials_cvode_create()
{
	static const MPT_INTERFACE_VPTR(solver) cVodeSol = {
		{ { cVodeUnref, cVodeRef }, cVodeConv, cVodeClone },
		cVodeReport,
		cVodeFcn,
		cVodeSolve
	};
	static const MPT_INTERFACE_VPTR(object) cVodeObj = {
		cVodeGet, cVodeSet
	};
	MPT_STRUCT(SundialsCVode) *cv;
	
	if (!(cv = malloc(sizeof(*cv)))) {
		return 0;
	}
	if (sundials_cvode_init(&cv->d) < 0) {
		free(cv);
		return 0;
	}
	memset(&cv->uf, 0, sizeof(cv->uf));
	CVodeSetUserData(cv->d.mem, &cv->d);
	cv->next = 0.0;
	
	cv->_sol._vptr = &cVodeSol;
	cv->_obj._vptr = &cVodeObj;
	
	return &cv->_sol;
}
