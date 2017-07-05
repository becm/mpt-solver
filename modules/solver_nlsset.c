/*!
 * set NLS parameter
 */

#include <stdlib.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_nlsset(MPT_SOLVER_NLS_STRUCT(parameters) *nls, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(solver_value) val;
	int32_t nv = 1, nr = 0;
	int ret;
	
	if (!src) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if ((ret = mpt_solver_value_set(&val, src)) < 0) {
		if ((ret = src->_vptr->conv(src, 'i', &nv)) < 0) {
			return ret;
		}
		ret = 0;
	}
	if (nv < 0) {
		return MPT_ERROR(BadValue);
	}
	if (!ret) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if ((ret = mpt_solver_next_int(&val, &nv)) < 0) {
		return ret;
	}
	if (!ret) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if (nv < 0) {
		return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_solver_next_int(&val, &nr)) < 0) {
		return ret;
	}
	if (!ret) {
		nls->nval = nv;
		nls->nres = nr;
		return 1;
	}
	if (nr < 0) {
		return MPT_ERROR(BadValue);
	}
	nls->nval = nv;
	nls->nres = nr;
	return 2;
}
