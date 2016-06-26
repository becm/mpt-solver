/*!
 * get vecpar properties
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "meta.h"

#include "solver.h"

extern int mpt_vecpar_set(double **ptr, int max, MPT_INTERFACE(metatype) *src)
{
	struct iovec tmp;
	double *dst, *old = *ptr;
	double v1 = 0.0, v2;
	int len, pos, curr;
	
	if (!src) {
		if (max < 0 && old) {
			free(old);
			*ptr = 0;
		}
		return 0;
	}
	if ((curr = src->_vptr->conv(src, MPT_value_toVector('d') | MPT_ENUM(ValueConsume), &tmp)) >= 0) {
		/* empty data */
		if (!curr) {
			if (!old) {
				return 0;
			}
			if (max < 0) {
				free(old);
				*ptr = 0;
				return 0;
			}
			for (pos = 0; pos < max; ++pos) {
				old[pos] = 0;
			}
			return 0;
		}
		len = tmp.iov_len/sizeof(double);
		
		/* need exact match */
		if (max > 0) {
			if (len != max) {
				return MPT_ERROR(BadValue);
			}
		}
		else if (!len) {
			if (old && max < 0) {
				free(old);
				*ptr = 0;
			}
			return len;
		}
		/* take available */
		if (!(dst = old)) {
			if (!(dst = malloc(len * sizeof(double)))) {
				return MPT_ERROR(BadOperation);
			}
			*ptr = dst;
		}
		/* copy/set existing values */
		if (tmp.iov_base) {
			memcpy(dst, tmp.iov_base, len * sizeof(double));
		} else {
			memset(dst, 0, len * sizeof(double));
		}
		return 1;
	}
	if ((curr = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &v1)) < 0) {
		return curr;
	}
	/* valid but empty data */
	if (!curr) {
		if (!old) {
			return 0;
		}
		if (max < 0) {
			free(old);
			*ptr = 0;
			return 0;
		}
		return 0;
	}
	/* single value only */
	if (max == 1 || (curr = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &v2)) <= 0) {
		/* require valid values only */
		if (curr < 0 && max >= 0) {
			return curr;
		}
		if (!(dst = old) && !(dst = calloc(max > 0 ? max : 1, sizeof(double)))) {
			return MPT_ERROR(BadOperation);
		}
		*dst = v1;
		if (!old) {
			*ptr = dst;
		}
		return 1;
	}
	/* start new vector */
	len = max > 0 ? max : 8;
	if (!(dst = malloc(len * sizeof(double)))) {
		return MPT_ERROR(BadOperation);
	}
	dst[0] = v1;
	dst[1] = v2;
	pos = 2;
	
	if (max == 2) {
		return 2;
	}
	/* read further data */
	while ((curr = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &v2))) {
		if (curr < 0) {
			/* need pure data or explicit size */
			if (max >= 0) {
				free(dst);
				return MPT_ERROR(BadType);
			}
			break;
		}
		/* increase vector space */
		if (pos >= len) {
			double *next;
			len += 8;
			if (!(next = realloc(dst, len * sizeof(double)))) {
				free(dst);
				return MPT_ERROR(BadOperation);
			}
		}
		dst[pos++] = v2;
		
		/* value number limit reached */
		if (pos == max) {
			break;
		}
	}
	/* replace existing data */
	if (max <= 0) {
		if (old) {
			free(old);
		}
	}
	/* copy to existing data */
	else if (old) {
		memcpy(old, dst, pos * sizeof(double));
		free(dst);
		return pos;
	}
	/* fill new data with zeros */
	if (max > pos) {
		for (len = pos; len < max; ++len) {
			dst[len] = 0;
		}
	}
	*ptr = dst;
	
	return pos;
}

extern int mpt_vecpar_get(const MPT_SOLVER_TYPE(dvecpar) *tol, MPT_STRUCT(value) *val)
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
