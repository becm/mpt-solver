/*!
 * get vecpar properties
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_vecpar_set(double *dest, long elem, long parts, MPT_INTERFACE(iterator) *it)
{
	struct iovec vec;
	uint32_t len;
	int ret;
	
	if (!parts) {
		/* get complete data segment */
		if ((ret = it->_vptr->get(it, MPT_value_toVector('d'), &vec)) >= 0) {
			if (!ret) {
				parts = 0;
			}
			else if ((ret = it->_vptr->advance(it)) < 0) {
				return ret;
			}
			else {
				if ((parts = vec.iov_len / sizeof(double)) > elem) {
					parts = elem;
				}
				ret = 1;
			}
			if (parts) {
				if (vec.iov_base) {
					memcpy(dest, vec.iov_base, parts * sizeof(double));
				} else {
					parts = 0;
				}
			}
			for ( ; parts < elem; ++parts) {
				dest[parts] = 0;
			}
			return ret;
		}
		parts = 0;
		/* get single elements */
		while (parts < elem) {
			if ((ret = it->_vptr->get(it, 'd', dest)) <= 0) {
				break;
			}
			if ((ret = it->_vptr->advance(it)) <= 0) {
				break;
			}
			++len;
			++dest;
			++parts;
		}
		while (parts < elem) {
			dest[parts++] = 0;
		}
		return len;
	}
	/* read initial value segments */
	len = 0;
	while (len < parts) {
		long curr;
		/* get profile segment */
		if ((ret = it->_vptr->get(it, MPT_value_toVector('d'), &vec)) < 0) {
			/* read constant profile values */
			for (curr = 0; curr < elem; ++curr) {
				if ((ret = it->_vptr->get(it, 'd', dest + curr)) <= 0
				 || (ret = it->_vptr->advance(it)) <= 0) {
					break;
				}
			}
			dest += elem;
			ret = curr;
			len = 1;
			break;
		}
		if (!ret || (ret = it->_vptr->advance(it)) < 0) {
			ret = len;
			break;
		}
		/* copy profile segment data */
		if (!vec.iov_base) {
			curr = 0;
		}
		else if ((curr = vec.iov_len / sizeof(double))) {
			if (curr > elem) {
				curr = elem;
			}
			memcpy(dest, vec.iov_base, curr * sizeof(double));
		}
		for ( ; curr < elem; ++curr) {
			dest[curr] = 0;
		}
		dest += elem;
		++len;
		if (!ret) {
			ret = len;
			break;
		}
	}
	/* repeat last profile segment */
	if (len) {
		while (len++ < parts) {
			memcpy(dest, dest - elem, elem * sizeof(double));
			dest += elem;
		}
	}
	return ret;
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
