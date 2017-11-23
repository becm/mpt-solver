/*!
 * generic solver interface for dVODE
 */

#include <stdlib.h>
#include <string.h>

#include "meta.h"

#include "module_functions.h"

#include "vode.h"

MPT_STRUCT(VodeData) {
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(vode) d;
	MPT_IVP_STRUCT(odefcn)  uf;
	
	double next;
};
/* reference interface */
static void vdFini(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(VodeData) *vd = (void *) ref;
	mpt_vode_fini(&vd->d);
	free(ref);
}
static uintptr_t vdAddref()
{
	return 0;
}
/* metatype interface */
static int vdConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	MPT_STRUCT(VodeData) *vd = (void *) mt;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&vd->_sol, &vd->_obj, type, ptr);
}
MPT_INTERFACE(metatype) *vdClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int vdReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	const MPT_STRUCT(VodeData) *vd = (void *) sol;
	if (!what && !out && !data) {
		return mpt_vode_get(&vd->d, 0);
	}
	return mpt_vode_report(&vd->d, what, out, data);
}
static int vdFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(VodeData) *vd = (void *) sol;
	return mpt_vode_ufcn(&vd->d, &vd->uf, type, ptr);
}
static int vdSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(VodeData) *vd = (void *) sol;
	return mpt_vode_step(&vd->d, vd->next);
}
/* object interface */
static int vdGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(VodeData) *vd = MPT_baseaddr(VodeData, obj, _obj);
	return mpt_vode_get(&vd->d, pr);
}
static int vdSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(VodeData) *vd = MPT_baseaddr(VodeData, obj, _obj);
	if (!pr) {
		if (!src) {
			int ret = mpt_vode_prepare(&vd->d);
			if (ret >= 0) {
				vd->next = vd->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		return mpt_solver_module_nextval(&vd->next, vd->d.t, src);
	}
	return mpt_vode_set(&vd->d, pr, src);
}

/*!
 * \ingroup mptSolverVode
 * \brief create VODE solver
 * 
 * Create dVODE solver instance with MPT interface.
 * 
 * \return VODE solver instance
 */
extern MPT_SOLVER(interface) *mpt_vode_create()
{
	static const MPT_INTERFACE_VPTR(object) vodeObj = {
		vdGet, vdSet
	};
	static const MPT_INTERFACE_VPTR(solver) vodeSol = {
		{ { vdFini, vdAddref }, vdConv, vdClone },
		vdReport,
		vdFcn,
		vdSolve
	};
	MPT_STRUCT(VodeData) *vd;
	
	if (!(vd = malloc(sizeof(*vd)))) {
		return 0;
	}
	mpt_vode_init(&vd->d);
	memset(&vd->uf, 0, sizeof(vd->uf));
	
	vd->_sol._vptr = &vodeSol;
	vd->_obj._vptr = &vodeObj;
	
	return &vd->_sol;
}
