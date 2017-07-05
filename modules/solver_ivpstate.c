/*!
 * set double values on vector
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

static double *setValues(double **y, uint32_t part, int32_t neqs, const double *from)
{
	struct iovec val = { 0, 0 };
	double *dest;
	size_t size;
	if (!(dest = mpt_solver_valloc(&val, part, neqs * sizeof(double)))) {
		return 0;
	}
	size = part * neqs * sizeof(double);
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

extern int mpt_solver_ivpstate(const MPT_SOLVER_IVP_STRUCT(parameters) *ivp, double *t, double **y, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec vec;
	double *dest, tmp;
	uint32_t neqs, len;
	int ret;
	
	if ((ret = ivp->neqs) < 1) {
		return MPT_ERROR(BadArgument);
	}
	neqs = ret;
	/* no iterator, need distinct conversion */
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		size_t part;
		if (t && (ret = src->_vptr->conv(src, 'd', &tmp)) < 0) {
			return MPT_ERROR(BadType);
		}
		if ((ret = src->_vptr->conv(src, MPT_value_toVector('d'), &vec)) < 0) {
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
		if ((ret = it->_vptr->get(it, 'd', &tmp)) < 0) {
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
	ret = mpt_solver_vecpar_set(dest, neqs, ret, it);
	
	if (ret < 0) {
		return ret;
	}
	if (t) {
		*t = tmp;
	}
	return 1 + ret;
}
