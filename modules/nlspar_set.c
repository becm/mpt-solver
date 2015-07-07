/*!
 * set NLS parameter
 */

#include <stdlib.h>

#include "solver.h"

extern int mpt_nlspar_set(MPT_SOLVER_STRUCT(nlspar) *nls, MPT_INTERFACE(source) *src)
{
	int32_t l1, l2;
	
	if (!src) {
		l1 = 0;
		if (nls->nval < 0) return MPT_ERROR(BadArgument);
		if (nls->nres < 0) return MPT_ERROR(BadValue);
		if (nls->nval != 1)  l1 |= 1;
		if (nls->nres != 0)  l1 |= 2;
		return l1;
	}
	if ((l1 = src->_vptr->conv(src, 'i', &l2)) > 0) {
		int32_t nv, res;
		if (l2 == 0) {
			return MPT_ERROR(BadValue);
		}
		nv = (l2 > 0) ? l2 : nls->nval;
		
		if ((l2 = src->_vptr->conv(src, 'i', &res)) == 0) {
			res = nv;
		} else if (l2 > 0) {
			if (res < nv) {
				return MPT_ERROR(BadValue);
			}
			l1 += l2;
		}
		nls->nval = nv;
		nls->nres = res;
	}
	return l1;
}
