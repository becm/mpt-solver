/*!
 * generic solver interface for RADAU
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "meta.h"

#include "radau.h"

MPT_STRUCT(RadauData) {
	MPT_SOLVER(interface) _sol;
	MPT_INTERFACE(object) _obj;
	MPT_SOLVER_STRUCT(radau) d;
	MPT_IVP_STRUCT(daefcn)   uf;
	double next;
};
/* reference interface */
static void rdFini(MPT_INTERFACE(reference) *ref)
{
	MPT_STRUCT(RadauData) *rd = (void *) ref;
	mpt_radau_fini(&rd->d);
	free(rd);
}
static uintptr_t rdAddref()
{
	return 0;
}
/* metatype interface */
static int rdConv(const MPT_INTERFACE(metatype) *mt, int type, void *ptr)
{
	const MPT_STRUCT(RadauData) *rd = (void *) mt;
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeMeta), MPT_ENUM(TypeObject), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeObject);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = &rd->_obj;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &rd->_sol;
		return MPT_ENUM(TypeObject);
	}
	return MPT_ERROR(BadType);
}
static MPT_INTERFACE(metatype) *rdClone(const MPT_INTERFACE(metatype) *mt)
{
	(void) mt;
	return 0;
}
/* solver interface */
static int rdReport(MPT_SOLVER(interface) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	const MPT_STRUCT(RadauData) *rd = (void *) sol;
	if (!what && !out && !data) {
		return mpt_radau_get(&rd->d, 0);
	}
	return mpt_radau_report(&rd->d, what, out, data);
}
static int rdFcn(MPT_SOLVER(interface) *sol, int type, const void *ptr)
{
	MPT_STRUCT(RadauData) *rd = (void *) sol;
	return mpt_radau_ufcn(&rd->d, &rd->uf, type, ptr);
}
static int rdSolve(MPT_SOLVER(interface) *sol)
{
	MPT_STRUCT(RadauData) *rd = (void *) sol;
	return mpt_radau_step(&rd->d, rd->next);
}
/* object interface */
static int rdGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	const MPT_STRUCT(RadauData) *rd = MPT_baseaddr(RadauData, obj, _obj);
	return mpt_radau_get(&rd->d, pr);
}
static int rdSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
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
		double end = rd->next;
		if (src && src->_vptr->conv(src, 'd', &end) < 0) {
			return MPT_ERROR(BadValue);
		}
		if (end < rd->d.t) {
			return MPT_ERROR(BadValue);
		}
		rd->next = end;
		return 0;
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
extern MPT_SOLVER(interface) *mpt_radau_create()
{
	static const MPT_INTERFACE_VPTR(object) radauObj = {
		rdGet, rdSet
	};
	static const MPT_INTERFACE_VPTR(solver) radauSol = {
		{ { rdFini, rdAddref }, rdConv, rdClone },
		rdReport,
		rdFcn,
		rdSolve
	};
	MPT_STRUCT(RadauData) *rd;
	
	if (!(rd = malloc(sizeof(*rd)))) {
		return 0;
	}
	mpt_radau_init(&rd->d);
	rd->d.ipar = memset(&rd->uf, 0, sizeof(rd->uf));
	
	rd->_sol._vptr = &radauSol;
	rd->_obj._vptr = &radauObj;
	
	return &rd->_sol;
}

