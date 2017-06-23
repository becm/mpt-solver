/*!
 * get vecpar properties
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

static ssize_t resizeVecpar(double **ptr, int elem, int parts)
{
	double *dst = *ptr;
	ssize_t max;
	
	if (parts < 1) {
		if (SIZE_MAX/2 /sizeof(*dst) < (size_t) elem) {
			return MPT_ERROR(BadValue);
		}
		max = elem;
	}
	else if (SIZE_MAX / parts / sizeof(*dst) < (size_t) elem) {
		return MPT_ERROR(BadValue);
	}
	else {
		max = elem * parts;
	}
	if (!(dst = realloc(dst, max * sizeof(*dst)))) {
		return MPT_ERROR(BadOperation);
	}
	*ptr = dst;
	return max;
}

static int setVecparDefault(double **ptr, int elem, int parts)
{
	double *dst;
	ssize_t max;
	
	if (elem < 0) {
		if ((dst = *ptr)) {
			free(dst);
			*ptr = 0;
		}
		return 0;
	}
	if (!elem) {
		return 0;
	}
	if ((max = resizeVecpar(ptr, elem, parts)) < 0) {
		return max;
	}
	dst = memset(*ptr, 0, max * sizeof(*dst));
	
	return parts > 0 ? parts : elem;
}
extern int mpt_solver_vecpar_set(double **ptr, int elem, int parts, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec tmp;
	double *dst = *ptr;
	ssize_t max;
	int pos, curr;
	
	if (!src) {
		return setVecparDefault(ptr, elem, parts);
	}
	if (!parts && (curr = src->_vptr->conv(src, MPT_value_toVector('d'), &tmp)) >= 0) {
		/* empty data */
		if (!curr) {
			if (elem < 1) {
				return MPT_ERROR(BadValue);
			}
			return setVecparDefault(ptr, elem, 0);
		}
		curr = tmp.iov_len / sizeof(double);
		
		/* need exact match */
		if (elem > 0) {
			if (tmp.iov_len != elem * sizeof(double)) {
				return MPT_ERROR(BadValue);
			}
		}
		else if (!curr) {
			if (dst && elem < 0) {
				free(dst);
				*ptr = 0;
			}
			return curr;
		}
		/* reserve target size */
		if (!(dst = realloc(dst, curr * sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
		*ptr = dst;
		/* copy/set existing values */
		if (tmp.iov_base) {
			memcpy(dst, tmp.iov_base, curr * sizeof(double));
		} else {
			memset(dst, 0, curr * sizeof(double));
		}
		return curr;
	}
	if (elem < 1 || parts < 0) {
		return MPT_ERROR(BadArgument);
	}
	if ((curr = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) < 0) {
		return curr;
	}
	/* valid but empty data */
	if (!curr || !it) {
		return setVecparDefault(ptr, elem, parts);
	}
	/* test double data conversion */
	if ((curr = it->_vptr->get(it, MPT_value_toVector('d'), &tmp)) < 0) {
		return MPT_ERROR(BadType);
	}
	if (!parts) {
		parts = 1;
	}
	if ((max = resizeVecpar(ptr, elem, parts)) < 0) {
		return max;
	}
	if (!(dst = *ptr)) {
		return MPT_ERROR(MissingData);
	}
	for (pos = 0; pos < parts; ++pos) {
		const double *from;
		int take;
		
		/* use part with good source data */
		if (curr && (from = tmp.iov_base)) {
			take = tmp.iov_len / sizeof(*from);
			if (take > elem) {
				take = elem;
			}
			memcpy(dst, from, take * sizeof(double));
		} else {
			take = 0;
		}
		if (elem > take) {
			memset(dst + take, 0, (elem - take) * sizeof(double));
		}
		dst += elem;
		
		/* advance to next element */
		if ((curr = it->_vptr->advance(it)) <= 0) {
			break;
		}
		/* setup next element content */
		curr = it->_vptr->get(it, MPT_value_toVector('d'), &tmp);
	}
	/* fill parts when iterator depleted */
	if (pos < parts) {
		max = (parts - pos) * sizeof(double);
		memset(dst, 0, max);
	}
	return pos;
}

extern int mpt_solver_vecpar_get(const MPT_SOLVER_TYPE(dvecpar) *tol, MPT_STRUCT(value) *val)
{
	int len = tol->base ? tol->d.len/sizeof(double) : 0;
	
	if (!val) {
		return len;
	}
	if (tol->base) {
		static const char fmt[2] = { MPT_value_toVector('d') };
		val->fmt = fmt;
		val->ptr = tol;
	} else {
		static const char fmt[2] = "d";
		val->fmt = fmt;
		val->ptr = &tol->d.val;
	}
	return len;
}
