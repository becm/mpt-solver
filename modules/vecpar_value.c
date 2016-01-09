/*!
 * get vecpar properties
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "solver.h"

extern int mpt_vecpar_set(MPT_TYPE(dvecpar) *val, MPT_INTERFACE(metatype) *src)
{
	struct iovec tmp;
	double v1 = 0.0, v2, *dest;
	int len, pos, full;
	
	if (!src) {
		if (val->base) {
			if (val->d.len/sizeof(double)) {
				val->d.val = *((double *) val->base);
			}
			free(val->base);
			val->base = 0;
		}
		return 0;
	}
	if ((len = src->_vptr->conv(src, MPT_value_toVector('d') | MPT_ENUM(ValueConsume), &tmp)) >= 0) {
		size_t elem = tmp.iov_len / sizeof(double);
		if (!elem) {
			dest = mpt_vecpar_alloc((struct iovec *) val, 0, 0);
			val->d.val = 0;
		}
		else if (!(dest = mpt_vecpar_alloc((struct iovec *) val, elem, sizeof(double)))) {
			return -1;
		}
		else {
			memcpy(val->base, tmp.iov_base, val->d.len);
		}
		return 1;
	}
	if ((len = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &v1)) < 0) {
		return len;
	}
	if (!len) v1 = 0.0;
	/* single value only */
	if (!len || !(pos = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &v2))) {
		dest = mpt_vecpar_alloc((struct iovec *) val, 0, 0);
		val->d.val = v1;
		return len ? 1 : 0;
	}
	if (pos < 0) {
		return pos;
	}
	/* start new vector */
	tmp.iov_base = 0;
	tmp.iov_len  = 0;
	
	full = 8;
	if (!(dest = mpt_vecpar_alloc(&tmp, full, sizeof(double)))) {
		return MPT_ERROR(BadOperation);
	}
	dest[0] = v1;
	dest[1] = v2;
	pos = 2;
	
	/* read further data */
	while ((len = src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &v2)) > 0) {
		/* increase vector space */
		if (pos >= full) {
			full += 8;
			if (!(dest = mpt_vecpar_alloc((struct iovec *) val, full, sizeof(double)))) {
				mpt_vecpar_alloc(&tmp, 0, 0);
				return MPT_ERROR(BadOperation);
			}
		}
		dest[pos++] = v2;
	}
	if (len < 0) {
		mpt_vecpar_alloc(&tmp, 0, 0);
		return MPT_ERROR(BadType);
	}
	/* shrink to required size */
	if (full > pos) {
		mpt_vecpar_alloc((struct iovec *) &tmp, pos, sizeof(double));
	}
	/* replace existing data */
	mpt_vecpar_alloc((struct iovec *) val, 0, 0);
	val->base = tmp.iov_base;
	val->d.len = tmp.iov_len;
	
	return pos;
}

extern int mpt_vecpar_get(const MPT_TYPE(dvecpar) *tol, MPT_STRUCT(value) *val)
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
