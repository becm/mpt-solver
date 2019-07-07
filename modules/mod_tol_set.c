/*!
 * MPT solver module helper function
 *   set vecpar data
 */

#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_module_tol_set(MPT_SOLVER_TYPE(dvecpar) *vec, MPT_INTERFACE(convertable) *src, double def)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec tmp;
	double *tol;
	long len;
	int ret;
	
	if (!src) {
		if ((tol = vec->_base)) {
			free(tol);
			vec->_base = 0;
		}
		vec->_d.val = def;
		return 0;
	}
	/* values from iterator */
	if ((ret = src->_vptr->convert(src, MPT_type_pointer(MPT_ENUM(TypeIterator)), &it)) >= 0) {
		double d;
		long reserved = 0;
		if (!ret || !it) {
			return mpt_solver_module_tol_set(vec, 0, def);
		}
		tol = 0;
		len = 0;
		while (1) {
			if ((ret = it->_vptr->get(it, 'd', &d)) < 0) {
				if (tol) {
					free(tol);
				}
				return ret;
			}
			if (!ret || (ret = it->_vptr->advance(it)) < 0) {
				break;
			}
			if (len >= reserved) {
				double *next;
				reserved += 8;
				if (!(next = realloc(tol, reserved * sizeof(*tol)))) {
					if (tol) {
						free(tol);
					}
					return MPT_ERROR(BadOperation);
				}
				tol = next;
			}
			tol[len++] = d;
			
			if (!ret) {
				break;
			}
		}
		if (len < 2) {
			if (len) {
				def = d;
				if (tol) {
					free(tol);
				}
			}
			return mpt_solver_module_tol_set(vec, 0, def);
		}
		if (vec->_base) {
			free(vec->_base);
		}
		vec->_base = tol;
		vec->_d.len = len * sizeof(*tol);
		return len;
	}
	if ((ret = src->_vptr->convert(src, MPT_type_vector('d'), &tmp)) < 0) {
		MPT_STRUCT(value) val = MPT_VALUE_INIT;
		len = 0;
		/* values from value content */
		if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeValue), &val)) < 0) {
			double d;
			if ((ret = src->_vptr->convert(src, 'd', &d)) < 0) {
				return ret;
			}
			return mpt_solver_module_tol_set(vec, 0, ret ? def : d);
		}
		else if (ret) {
			if (!val.fmt) {
				return MPT_ERROR(BadValue);
			}
			/* consume matching elements */
			while (*val.fmt++ == 'd') {
				++len;
			}
		}
		tmp.iov_base = (double *) val.ptr;
		
		if ((ret = len) < 2) {
			if (len && val.ptr) {
				def = *((double *) val.ptr);
			}
			mpt_solver_module_tol_set(vec, 0, def);
			return ret;
		}
	}
	/* values from double vector */
	else {
		len = tmp.iov_len / sizeof(*tol);
		if (len < 2) {
			if (len && tmp.iov_base) {
				def = *((double *) tmp.iov_base);
			}
			return mpt_solver_module_tol_set(vec, 0, def);
		}
		ret = 0;
	}
	/* reserve new tolerance data */
	tol = vec->_base;
	if (!(tol = realloc(tol, len * sizeof(*tol)))) {
		return MPT_ERROR(BadOperation);
	}
	vec->_base = tol;
	vec->_d.len = len * sizeof(*tol);
	
	/* fill required size with default values */
	if (!tmp.iov_base) {
		int pos;
		for (pos = 0; pos < len; ++pos) {
			tol[pos] = def;
		}
	}
	/* copy available tolerance elements */
	else {
		memcpy(tol, tmp.iov_base, len * sizeof(*tol));
	}
	return ret;
}
