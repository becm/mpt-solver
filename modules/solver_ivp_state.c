/*!
 * set double values on vector
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

#ifndef MPT_SOLVER_MODULE_DATA_RESIZE
# define MPT_SOLVER_MODULE_DATA_RESIZE setValues
static double *setValues(MPT_SOLVER_MODULE_DATA_CONTAINER *y, uint32_t part, int32_t neqs, const MPT_SOLVER_MODULE_DATA_TYPE *from)
{
	struct iovec val = { 0, 0 };
	MPT_SOLVER_MODULE_DATA_TYPE *dest;
	size_t size;
	if (!(dest = mpt_solver_valloc(&val, part, neqs * sizeof(*dest)))) {
		return 0;
	}
	size = part * neqs * sizeof(*dest);
	if (*y) {
		free(*y);
	}
	if (from) {
		*y = memcpy(dest, from, size);
	} else {
		*y = memset(dest, 0, size);
	}
	return dest;
}
#endif

extern int MPT_SOLVER_MODULE_FCN(ivp_state)(const MPT_IVP_STRUCT(parameters) *ivp, MPT_SOLVER_MODULE_DATA_TYPE *t, MPT_SOLVER_MODULE_DATA_CONTAINER *y, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec vec;
	MPT_SOLVER_MODULE_DATA_TYPE *dest, tmp;
	uint32_t neqs, len;
	int ret;
	
	if ((ret = ivp->neqs) < 1) {
		return MPT_ERROR(BadArgument);
	}
	neqs = ret;
	/* no iterator, need distinct conversion */
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		size_t part;
		if (t && (ret = src->_vptr->conv(src, MPT_SOLVER_MODULE_DATA_ID, &tmp)) < 0) {
			return MPT_ERROR(BadType);
		}
		if ((ret = src->_vptr->conv(src, MPT_value_toVector(MPT_SOLVER_MODULE_DATA_ID), &vec)) < 0) {
			return MPT_ERROR(BadType);
		}
		if (!ret) {
			vec.iov_base = 0;
			part = 0;
		} else {
			part = vec.iov_len / sizeof(double);
		}
		if (ivp->pint) {
			if ((part /= neqs) < (ivp->pint + 1)) {
				return MPT_ERROR(BadValue);
			}
			if (!(dest = setValues(y, ivp->pint + 1, neqs, vec.iov_base))) {
				return MPT_ERROR(BadOperation);
			}
		}
		else if (!(dest = setValues(y, 1, neqs, 0))) {
			return MPT_ERROR(BadOperation);
		}
		else {
			const double *src;
			size_t i = 0;
			if ((src = vec.iov_base)) {
				while (i < part) {
					dest[i] = src[i];
					++i;
				}
			}
		}
		if (t) {
			*t = tmp;
		}
		return 0;
	}
	/* require real iterator */
	if (!ret || !it) {
		return MPT_ERROR(BadValue);
	}
	/* consume time value */
	if (!t) {
		len = 0;
		tmp = 0;
	} else {
		if ((ret = it->_vptr->get(it, MPT_SOLVER_MODULE_DATA_ID, &tmp)) < 0) {
			return ret;
		}
		len = ret ? 1 : 0;
		if (len && (ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
	}
	/* reserve total state size */
	if (!(dest = setValues(y, ivp->pint + 1, ivp->neqs, 0))) {
		return MPT_ERROR(BadOperation);
	}
	if (!len) {
		if (t) {
			*t = tmp;
		}
		return len;
	}
	/* get segment content */
	ret = ivp->pint ? ivp->pint + 1 : 0;
	ret = MPT_SOLVER_MODULE_FCN(data_set)(dest, neqs, ret, it);
	
	if (ret < 0) {
		return ret;
	}
	if (t) {
		*t = tmp;
	}
	return 1 + ret;
}
