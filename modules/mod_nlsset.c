/*!
 * MPT solver module helper function
 *   set NLS parameter and residual count
 */

#include <stdlib.h>

#include "module.h"

#include "../solver.h"

extern int mpt_solver_module_nlsset(MPT_NLS_STRUCT(parameters) *nls, const MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(module_value) val = MPT_MODULE_VALUE_INIT;
	int32_t nv = 1, nr = 0;
	int ret;
	
	if (!src) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	if ((ret = mpt_module_value_init(&val, src)) < 0) {
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
	if ((ret = mpt_module_value_int(&val, &nv)) < 0) {
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
	if ((ret = mpt_module_value_int(&val, &nr)) < 0) {
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
