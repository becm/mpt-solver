/*!
 * generic solver interface for RADAU
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "meta.h"

#include "radau.h"

static void rdFini(MPT_INTERFACE(unrefable) *gen)
{
	mpt_radau_fini((MPT_SOLVER_STRUCT(radau) *) (gen+1));
	free(gen);
}
static uintptr_t rdAddref()
{
	return 0;
}
static int rdGet(const MPT_INTERFACE(object) *obj, MPT_STRUCT(property) *pr)
{
	return mpt_radau_get((MPT_SOLVER_STRUCT(radau) *) (obj + 1), pr);
}
static int rdSet(MPT_INTERFACE(object) *obj, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	return _mpt_radau_set((MPT_SOLVER_STRUCT(radau) *) (obj + 1), pr, src);
}

static int rdReport(MPT_SOLVER(generic) *sol, int what, MPT_TYPE(PropertyHandler) out, void *data)
{
	if (!what && !out && !data) {
		return MPT_SOLVER_ENUM(DAE) | MPT_SOLVER_ENUM(PDE);
	}
	return mpt_radau_report((MPT_SOLVER_STRUCT(radau) *) (sol + 1), what, out, data);
}
static int rdFcn(MPT_SOLVER(generic) *sol, int type, const void *ptr)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) (sol + 1);
	return mpt_radau_ufcn(rd, (void *) (rd + 1), type, ptr);
}

static const MPT_INTERFACE_VPTR(solver) radauCtl = {
	{ { rdFini }, rdAddref, rdGet, rdSet },
	rdReport,
	rdFcn
};


extern int _mpt_radau_set(MPT_SOLVER_STRUCT(radau) *rd, const char *pr, const MPT_INTERFACE(metatype) *src)
{
	if (!pr) {
		if (!src) {
			return mpt_radau_prepare(rd);
		}
	} else if (src && pr[0] == 't' && pr[1] == 0) {
		double end;
		int ret = src->_vptr->conv(src, 'd', &end);
		
		if (ret < 0) return ret;
		if (!ret) return MPT_ERROR(BadValue);
		return mpt_radau_step(rd, end);
	}
	return mpt_radau_set(rd, pr, src);
}

extern MPT_SOLVER(generic) *mpt_radau_create()
{
	MPT_SOLVER(generic) *sol;
	MPT_SOLVER_STRUCT(radau) *rd;
	MPT_IVP_STRUCT(daefcn) *fcn;
	
	if (!(sol = malloc(sizeof(*sol) + sizeof(*rd) + sizeof(*fcn)))) {
		return 0;
	}
	rd = (MPT_SOLVER_STRUCT(radau *)) (sol + 1);
	mpt_radau_init(rd);
	
	rd->ipar = memset(rd + 1, 0, sizeof(*fcn));
	
	sol->_vptr = &radauCtl;
	
	return sol;
}

