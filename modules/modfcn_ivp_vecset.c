/*!
 * MPT solver module helper function
 *   replace solver data matching IVP dimensions
 */

#include <stdlib.h>

#include "meta.h"

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(ivp_vecset)(const MPT_IVP_STRUCT(parameters) *ivp, MPT_SOLVER_MODULE_DATA_CONTAINER *val, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	MPT_SOLVER_MODULE_DATA_TYPE *ptr;
	long max;
	int ret;
	
	if (ivp->neqs < 1) {
		return MPT_ERROR(BadArgument);
	}
	it = 0;
	if (src && (ret = src->_vptr->conv(src, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it)) < 0) {
		return ret;
	}
	max = ivp->pint + 1;
	if (!(ptr = MPT_SOLVER_MODULE_FCN(data_new)(val, max * ivp->neqs, 0))) {
		return MPT_ERROR(BadOperation);
	}
	if (!ivp->pint) {
		max = 0;
	}
	return MPT_SOLVER_MODULE_FCN(data_set)(ptr, ivp->neqs, max, it);
}
