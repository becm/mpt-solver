/*!
 * set NLS parameter
 */

#include <stdlib.h>

#include "meta.h"

#include "solver.h"

extern int mpt_nlspar_set(MPT_SOLVER_NLS_STRUCT(parameters) *nls, MPT_INTERFACE(metatype) *src)
{
	int32_t nv, nr;
	int res;
	
	if (!src) {
		nls->nval = 1;
		nls->nres = 0;
		return 0;
	}
	if ((res = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &nv)) < 0) {
		return res;
	}
	if (!res) {
		nls->nval = 1;
		nls->nres = 0;
		return 0;
	}
	if (nv < 0) {
		return MPT_ERROR(BadValue);
	}
	if ((res = src->_vptr->conv(src, 'i' | MPT_ENUM(ValueConsume), &nr)) < 0) {
		return res;
	}
	if (!res) {
		nls->nval = nv;
		nls->nres = nv;
		return 1;
	}
	if (nr < 0 || (nr && nr < nv)) {
		return MPT_ERROR(BadValue);
	}
	nls->nval = nv;
	nls->nres = nr;
	
	return 2;
}
