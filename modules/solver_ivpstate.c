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
	uint32_t part, neqs, len;
	int ret;
	
	if ((neqs = ivp->neqs) < 1) {
		return MPT_ERROR(BadArgument);
	}
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
		}
		else if (!(part = vec.iov_len / sizeof(double) / neqs) || (part - 1) < ivp->pint) {
			return MPT_ERROR(BadValue);
		}
		if (!(dest = setValues(y, ivp->pint + 1, neqs, vec.iov_base))) {
			return MPT_ERROR(BadOperation);
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
			return MPT_ERROR(BadValue);
		}
		len = ret ? 1 : 0;
		if (len && (ret = it->_vptr->advance(it)) < 0) {
			return MPT_ERROR(BadOperation);
		}
	}
	/* reserve total state size */
	if (!(dest = setValues(y, ivp->pint + 1, ivp->neqs, 0))) {
		return MPT_ERROR(BadOperation);
	}
	if (!ret) {
		if (t) {
			*t = tmp;
		}
		return len;
	}
	len = 0;
	/* read single vector or values */
	if (!ivp->pint) {
		if ((ret = it->_vptr->get(it, MPT_value_toVector('d'), &vec)) >= 0) {
			if (!ret) {
				part = 0;
			}
			else if ((ret = it->_vptr->advance(it)) < 0) {
				return ret;
			}
			else {
				if ((part = vec.iov_len / sizeof(double)) > neqs) {
					part = ivp->neqs;
				}
				++len;
			}
			if (part) {
				if (vec.iov_base) {
					memcpy(dest, vec.iov_base, part * sizeof(double));
				} else {
					memset(dest, 0, part * sizeof(double));
				}
			}
			for ( ; part < neqs; ++part) {
				dest[part] = 0;
			}
			if (t) {
				*t = tmp;
			}
			return 1 + len;
		}
		part = 0;
		while (part < neqs) {
			if ((ret = it->_vptr->get(it, 'd', dest)) <= 0) {
				break;
			}
			if ((ret = it->_vptr->advance(it)) <= 0) {
				break;
			}
			++len;
			++dest;
			++part;
		}
		while (part < neqs) {
			dest[part++] = 0;
		}
		if (t) {
			*t = tmp;
		}
		return len + 1;
	}
	/* read initial value segments */
	while (len <= ivp->pint) {
		part = 0;
		/* get profile segment */
		if ((ret = it->_vptr->get(it, MPT_value_toVector('d'), &vec)) < 0) {
			/* read contant profile values */
			for (part = 0; part < neqs; ++part) {
				if ((ret = it->_vptr->get(it, 'd', dest + part)) <= 0
				 || (ret = it->_vptr->advance(it)) <= 0) {
					break;
				}
			}
			dest += neqs;
			ret = part;
			len = 1;
			break;
		}
		if (!ret || (ret = it->_vptr->advance(it)) < 0) {
			ret = len;
			break;
		}
		/* copy profile segment data */
		if ((part = vec.iov_len / sizeof(double))) {
			if (part > neqs) {
				part = neqs;
			}
			if (vec.iov_base) {
				memcpy(dest, vec.iov_base, part * sizeof(double));
			}
		}
		dest += ivp->neqs;
		++len;
		if (!ret) {
			ret = len;
			break;
		}
	}
	/* repeat last profile segment */
	if (len) {
		while (len++ <= ivp->pint) {
			memcpy(dest, dest - neqs, neqs * sizeof(double));
			dest += neqs;
		}
	}
	if (t) {
		*t = tmp;
	}
	return 1 + ret;
}
