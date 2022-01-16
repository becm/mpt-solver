/*!
 * MPT solver module helper function
 *   set vecpar data
 */

#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "types.h"
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
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) >= 0) {
		double d;
		long reserved = 0;
		if (!ret || !it) {
			return mpt_solver_module_tol_set(vec, 0, def);
		}
		tol = 0;
		len = 0;
		while (1) {
			const MPT_STRUCT(value) *val;
			
			if (!(val = it->_vptr->value(it))
			 || !val->type
			 || !val->ptr) {
				break;
			}
			if (val->type == 'd') {
				d = *((const double *) val->ptr);
			}
			else if (!MPT_type_isConvertable(val->type)) {
				break;
			}
			else {
				MPT_INTERFACE(convertable) *conv = *((void * const *) val->ptr);
				
				if (!conv) {
					break;
				}
				if ((ret = conv->_vptr->convert(conv, 'd', &d)) < 0) {
					return ret;
				}
			}
			if ((ret = it->_vptr->advance(it)) < 0) {
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
	if ((ret = src->_vptr->convert(src, MPT_type_toVector('d'), &tmp)) < 0) {
		double d;
		len = 0;
		/* scalar tolerance value */
		if ((ret = src->_vptr->convert(src, 'd', &d)) < 0) {
			return ret;
		}
		return mpt_solver_module_tol_set(vec, 0, ret ? def : d);
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
