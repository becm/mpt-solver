/*!
 * generic solver interface for LIMEX
 */

#include <stdlib.h>
#include <string.h>

#include "module.h"

#include "limex.h"

static MPT_SOLVER_STRUCT(limex) lxGlob;
static MPT_IVP_STRUCT(daefcn) lxGlobFcn;
static double lxNext = 0.0;
static int lxReserved = 0;

extern MPT_SOLVER_STRUCT(limex) *mpt_limex_global()
{
	if (lxReserved) return &lxGlob;
	
	mpt_limex_init(&lxGlob);
	lxGlob.ufcn = memset(&lxGlobFcn, 0, sizeof(lxGlobFcn));
	lxReserved = 1;
	return &lxGlob;
}
/* objet interface */
static int lxGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	(void) obj;
	return mpt_limex_get(&lxGlob, pr);
}
static int lxSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	(void) obj;
	if (!pr) {
		if (!src) {
			return mpt_limex_prepare(&lxGlob);
		}
	} else if (pr[0] == 't' && !pr[1]) {
		return mpt_solver_module_nextval(&lxNext, lxGlob.t, src);
	}
	return mpt_limex_set(&lxGlob, pr, src);
}
static const MPT_INTERFACE_VPTR(object) limexObj = {
	lxGet, lxSet
};
/* reference interface */
static void lxFini()
{
	mpt_limex_fini(&lxGlob);
}
static uintptr_t lxAddref()
{
	return 0;
}
/* metatype interface */
static int lxConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr);
static MPT_INTERFACE(metatype) *lxClone()
{
	return 0;
}
/* solver interface */
static int lxReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	(void) sol;
	if (!what && !out && !data) {
		return mpt_limex_get(&lxGlob, 0);
	}
	return mpt_limex_report(&lxGlob, what, out, data);
}
static int lxFcn(MPT_SOLVER(interface) *sol, int type, const void *par)
{
	(void) sol;
	return mpt_limex_ufcn(&lxGlob, &lxGlobFcn, type, par);
}
static int lxSolve(MPT_SOLVER(interface) *sol)
{
	(void) sol;
	return mpt_limex_step(&lxGlob, lxNext);
}
static const MPT_INTERFACE_VPTR(solver) limexSol = {
	{ { lxFini, lxAddref }, lxConv, lxClone },
	lxReport,
	lxFcn,
	lxSolve
};
static MPT_STRUCT(module_generic) lxGlobSolver = {
	{ &limexSol.meta },
	{ &limexObj }
};

static int lxConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	(void) mt;
	return mpt_module_generic_conv(&lxGlobSolver, type, ptr);
}

/*!
 * \ingroup mptLimex
 * \brief create LIMEX solver
 * 
 * Create LIMEX solver instance with MPT interface.
 * 
 * \return LIMEX solver instance
 */
extern MPT_SOLVER(interface) *mpt_limex_create()
{
	if (lxReserved) return 0;
	mpt_limex_global();
	return (void *) &lxGlobSolver._mt;
}
