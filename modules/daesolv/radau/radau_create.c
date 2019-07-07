/*!
 * generic solver interface for RADAU
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "radau.h"

#include "meta.h"

#include "module_functions.h"

MPT_STRUCT(RadauData) {
	MPT_INTERFACE(metatype) _mt;
	
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	
	MPT_SOLVER_STRUCT(radau) d;
	MPT_IVP_STRUCT(daefcn)   uf;
	
	double next;
};
/* convertable interface */
static int rdConv(MPT_INTERFACE(convertable) *sol, int type, void *ptr)
{
	MPT_STRUCT(RadauData) *rd = (void *) sol;
	return MPT_SOLVER_MODULE_FCN(solver_conv)(&rd->_sol, &rd->_obj, type, ptr);
}
/* metatype interface */
static void rdFini(MPT_INTERFACE(metatype) *mt)
{
	MPT_STRUCT(RadauData) *rd = (void *) mt;
	mpt_radau_fini(&rd->d);
	free(rd);
}
static uintptr_t rdAddref()
{
	return 0;
}
static MPT_INTERFACE(metatype) *rdClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int rdReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(property_handler) out, void *data)
{
	const MPT_STRUCT(RadauData) *rd = MPT_baseaddr(RadauData, sol, _sol);
	if (!what && !out && !data) {
		return mpt_radau_get(&rd->d, 0);
	}
	return mpt_radau_report(&rd->d, what, out, data);
}
static int rdFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(RadauData) *rd = MPT_baseaddr(RadauData, sol, _sol);
	return mpt_radau_ufcn(&rd->d, &rd->uf, type, ptr);
}
static int rdSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(RadauData) *rd = MPT_baseaddr(RadauData, sol, _sol);
	return mpt_radau_step(&rd->d, rd->next);
}
/* object interface */
static int rdGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(RadauData) *rd = MPT_baseaddr(RadauData, obj, _obj);
	return mpt_radau_get(&rd->d, pr);
}
static int rdSet(MPT_INTERFACE(object) *obj, const char *pr, MPT_INTERFACE(convertable) *src)
{
	MPT_STRUCT(RadauData) *rd = MPT_baseaddr(RadauData, obj, _obj);
	if (!pr) {
		if (!src) {
			int ret = mpt_radau_prepare(&rd->d);
			if (ret >= 0) {
				rd->next = rd->d.t;
			}
			return ret;
		}
	} else if (pr[0] == 't' && pr[1] == 0) {
		return mpt_solver_module_nextval(&rd->next, rd->d.t, src);
	}
	return mpt_radau_set(&rd->d, pr, src);
}

/*!
 * \ingroup mptDaesolvRadau
 * \brief create RADAU solver
 * 
 * Create RADAU solver instance with MPT interface.
 * 
 * \return RADAU solver instance
 */
extern MPT_INTERFACE(metatype) *mpt_radau_create()
{
	static const MPT_INTERFACE_VPTR(object) radauObj = {
		rdGet, rdSet
	};
	static const MPT_INTERFACE_VPTR(solver) radauSol = {
		rdReport,
		rdFcn,
		rdSolve
	};
	static const MPT_INTERFACE_VPTR(metatype) radauMeta = {
		{ rdConv },
		rdFini,
		rdAddref,
		rdClone
	};
	MPT_STRUCT(RadauData) *rd;
	
	if (!(rd = malloc(sizeof(*rd)))) {
		return 0;
	}
	mpt_radau_init(&rd->d);
	rd->d.ipar = memset(&rd->uf, 0, sizeof(rd->uf));
	
	rd->_mt._vptr = &radauMeta;
	
	rd->_sol._vptr = &radauSol;
	rd->_obj._vptr = &radauObj;
	
	return &rd->_mt;
}

