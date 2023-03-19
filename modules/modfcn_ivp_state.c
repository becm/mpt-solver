/*!
 * MPT solver module helper function
 *   set initial IVP state
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "types.h"
#include "meta.h"

#include "../solver.h"

extern int MPT_SOLVER_MODULE_FCN(ivp_state)(const MPT_IVP_STRUCT(parameters) *ivp, MPT_SOLVER_MODULE_DATA_TYPE *t, MPT_SOLVER_MODULE_DATA_CONTAINER *y, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	MPT_SOLVER_MODULE_DATA_TYPE *dest, tmp;
	struct iovec vec;
	size_t size, neqs;
	int ret, done;
	
	if (ivp->neqs < 1) {
		return MPT_ERROR(BadArgument);
	}
	vec.iov_base = 0;
	vec.iov_len = 0;
	it = 0;
	/* pure state data */
	if ((ret = src->_vptr->convert(src, MPT_type_toVector(MPT_SOLVER_MODULE_DATA_ID), &vec)) >= 0) {
		ret = 0;
	}
	/* require state content */
	else if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) < 0 || !it) {
		return MPT_ERROR(BadType);
	}
	/* require time value */
	else if (t) {
		tmp = *t;
		ret = 0;
		
		if ((ret = src->_vptr->convert(src, MPT_SOLVER_MODULE_DATA_ID, &tmp)) < 0) {
			const MPT_STRUCT(value) *val;
			if (!(val = it->_vptr->value(it))
			 || !val->_addr) {
				return MPT_ERROR(BadValue);
			}
			if (val->_type == MPT_SOLVER_MODULE_DATA_ID) {
				tmp = *((MPT_SOLVER_MODULE_DATA_TYPE *) val->_addr);
			}
			else if (MPT_type_isConvertable(val->_type)) {
				MPT_INTERFACE(convertable) *conv = *((void * const *) val->_addr);
				if (!conv) {
					return MPT_ERROR(BadValue);
				}
				if ((ret = conv->_vptr->convert(conv, MPT_SOLVER_MODULE_DATA_ID, &tmp)) < 0) {
					return ret;
				}
			}
			else {
				return MPT_ERROR(BadType);
			}
			/* consume time value */
			if ((ret = it->_vptr->advance(it)) <= 0) {
				return MPT_ERROR(MissingData);
			}
			ret = 1;
		}
	}
	neqs = ivp->neqs;
	/* copy existing compatible data */
	if (!it) {
		const MPT_SOLVER_MODULE_DATA_TYPE *src;
		size_t part;
		
		if (ret < 2) {
			src  = 0;
			part = 0;
		} else {
			src  = vec.iov_base;
			part = vec.iov_len / sizeof(double);
		}
		/* require full init size */
		if (ivp->pint) {
			size = ivp->pint + 1;
			if ((part /= neqs) < size) {
				return MPT_ERROR(BadValue);
			}
			if (!(dest = MPT_SOLVER_MODULE_FCN(data_new)(y, size * neqs, src))) {
				return MPT_ERROR(BadOperation);
			}
		}
		/* use zero values for unset data */
		else if (!(dest = MPT_SOLVER_MODULE_FCN(data_new)(y, neqs, 0))) {
			return MPT_ERROR(BadOperation);
		}
		else if (src && part) {
			size_t i = 0;
			if (part > neqs) {
				part = neqs;
			}
			while (i < part) {
				dest[i] = src[i];
				++i;
			}
		}
		if (t) {
			*t = tmp;
		}
		return ret;
	}
	/* reserve total state size */
	size = ivp->pint + 1;
	if (!(dest = MPT_SOLVER_MODULE_FCN(data_new)(y, size * neqs, 0))) {
		return MPT_ERROR(BadOperation);
	}
	/* get segment content */
	done = MPT_SOLVER_MODULE_FCN(data_set)(dest, neqs, ivp->pint ? size : 0, it);
	
	if (done < 0) {
		return done;
	}
	if (t) {
		*t = tmp;
	}
	return ret + done;
}
