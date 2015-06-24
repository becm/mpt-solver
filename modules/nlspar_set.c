/*!
 * set NLS parameter
 */

#include <stdlib.h>

#include "solver.h"

extern int mpt_nlspar_set(MPT_SOLVER_STRUCT(nlspar) *nls, MPT_INTERFACE(source) *src)
{
	int l1, l2;
	
	if (!src) {
		l1 = 0;
		if (nls->nval < 0) return -1;
		if (nls->nres < 0) return -2;
		if (nls->nval != 1)  l1 |= 1;
		if (nls->nres != 0)  l1 |= 2;
		return l1;
	}
	if ((l1 = src->_vptr->conv(src, 'i', &l2)) > 0) {
		int l3;
		if (l2 == 0) return -3;
		if (l2 > 0) nls->nval = l2;
		
		if ((l2 = src->_vptr->conv(src, 'i', &l3)) < 0) l2 = 0;
		else if (!l2) nls->nres = nls->nval;
		else if (l3 >= 0) nls->nres = l3;
	}
	return l1+l2;
}
