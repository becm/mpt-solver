/*!
 * generic solver interface for MEBDFI
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "meta.h"

#include "mebdfi.h"

MPT_STRUCT(MebdfiData) {
	MPT_SOLVER(generic) _gen;
	MPT_SOLVER_STRUCT(mebdfi) d;
	MPT_IVP_STRUCT(daefcn)    uf;
	double next;
};
/* reference interface */
static void meFini(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(MebdfiData) *md = (void *) ref;
	mpt_mebdfi_fini(&md->d);
	free(md);
}
static uintptr_t meAddref()
{
	return 0;
}
/* metatype interface */
static int meConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(MebdfiData) *md = (void *) mt;
	return mpt_solver_generic_conv(&md->_gen, type, ptr);
}
static MPT_INTERFACE(metatype) *meClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int meReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	const MPT_STRUCT(MebdfiData) *md = (void *) sol;
	if (!what && !out && !data) {
		return mpt_mebdfi_get(&md->d, 0);
	}
	return mpt_mebdfi_report(&md->d, what, out, data);
}
static int meFcn(MPT_SOLVER(interface) *sol, int type, const void *par)
{
	MPT_STRUCT(MebdfiData) *md = (void *) sol;
	return mpt_mebdfi_ufcn(&md->d, &md->uf, type, par);
}
static int meSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(MebdfiData) *md = (void *) sol;
	return mpt_mebdfi_step(&md->d, md->next);
}
/* object interface */
static int meGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(MebdfiData) *md = MPT_baseaddr(MebdfiData, obj, _gen._obj);
	return mpt_mebdfi_get(&md->d, pr);
}
static int meSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(MebdfiData) *md = MPT_baseaddr(MebdfiData, obj, _gen._obj);
	if (!pr) {
		if (!src) {
			int ret = mpt_mebdfi_prepare(&md->d);
			if (ret >= 0) {
				md->next = md->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		return mpt_solver_ivp_settime(&md->next, md->d.t, src);
	}
	return mpt_mebdfi_set(&md->d, pr, src);
}

/*!
 * \ingroup mptDaesolvMebdfi
 * \brief create MEBDFI solver
 * 
 * Create MEBDFI solver instance with MPT interface.
 * 
 * \return MEBDFI solver instance
 */
extern MPT_SOLVER(interface) *mpt_mebdfi_create()
{
	static const MPT_INTERFACE_VPTR(object) mebdfiObj = {
		meGet, meSet
	};
	static const MPT_INTERFACE_VPTR(solver) mebdfiSol = {
		{ { meFini, meAddref }, meConv, meClone },
		meReport,
		meFcn,
		meSolve
	};
	MPT_STRUCT(MebdfiData) *md;
	
	if (!(md = malloc(sizeof(*md)))) {
		return 0;
	}
	mpt_mebdfi_init(&md->d);
	md->d.ipar = memset(&md->uf, 0, sizeof(md->uf));
	
	md->_gen._sol._vptr = &mebdfiSol;
	md->_gen._obj._vptr = &mebdfiObj;
	
	return &md->_gen._sol;
}

