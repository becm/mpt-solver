/*!
 * MPT solver module helper function
 *   set NLS parameter and residual count
 */

#include "../solver.h"

extern int mpt_solver_module_value_nls(MPT_STRUCT(property) *pr, const MPT_NLS_STRUCT(parameters) *par)
{
	mpt_solver_module_value_int(pr, par ? &par->nval : 0);
	return par && (par->nval != 1) ? 1 : 0;
}


extern int mpt_solver_module_nlsset(MPT_NLS_STRUCT(parameters) *nls, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	int32_t nv = 1, nr = 0;
	int ret;
	
	if (!src) {
		nls->nval = nv;
		nls->nres = nr;
		return 0;
	}
	it = 0;
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) < 0) {
		if ((ret = src->_vptr->convert(src, 'i', &nv)) < 0) {
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
	if (!it) {
		return MPT_ERROR(BadValue);
	}
	if ((ret = mpt_solver_module_consume_value(it, 'i', &nv, sizeof(nv))) < 0) {
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
	if ((ret = mpt_solver_module_consume_value(it, 'i', &nr, sizeof(nr))) < 0) {
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
