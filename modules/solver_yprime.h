/*!
 * generic user functions dDASSL solver instance
 */

#include <stdlib.h>

#include "meta.h"

static int _mpt_solver_yprime_set(const MPT_IVP_STRUCT(parameters) *ivp, double **yp, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it = 0;
	long max;
	double *ptr;
	int ret;
	
	if (src && (ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		return ret;
	}
	if ((max = ivp->pint)) {
		++max;
	}
	ptr = *yp;
	if (!(ptr = realloc(ptr, max * ivp->neqs * sizeof(*yp)))) {
		return MPT_ERROR(BadOperation);
	}
	*yp = ptr;
	if (!ivp->pint) {
		max = 0;
	}
	return mpt_solver_vecpar_set(ptr, ivp->neqs, max, ret ? it : 0);
}
