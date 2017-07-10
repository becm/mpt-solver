/*!
 * generic user functions dDASSL solver instance
 */

#include <stdlib.h>

#include "meta.h"

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(ivp_vecset)(const MPT_IVP_STRUCT(parameters) *ivp, MPT_SOLVER_MODULE_DATA_CONTAINER *val, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it = 0;
	long max;
	MPT_SOLVER_MODULE_DATA_TYPE *ptr;
	int ret;
	
	if (src && (ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		return ret;
	}
	if ((max = ivp->pint)) {
		++max;
	}
	if (!(ptr = MPT_SOLVER_MODULE_FCN(data_new)(val, max * ivp->neqs, 0))) {
		return MPT_ERROR(BadOperation);
	}
	if (!ivp->pint) {
		max = 0;
	}
	return MPT_SOLVER_MODULE_FCN(data_set)(ptr, ivp->neqs, max, ret ? it : 0);
}
