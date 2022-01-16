/*!
 * MPT solver module helper function
 *   set IVP equotation count and grid size/data
 */

#include <stdlib.h>

#include <sys/uio.h>

#include "types.h"
#include "meta.h"

#include "../solver.h"

extern int mpt_solver_module_value_ivp(MPT_STRUCT(value) *val, const MPT_IVP_STRUCT(parameters) *par)
{
	mpt_solver_module_value_int(val, par ? &par->neqs : 0);
	return par && (par->neqs != 1 || par->pint) ? 1 : 0;
}

extern int mpt_solver_module_ivpset(MPT_IVP_STRUCT(parameters) *ivp, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec grid;
	int32_t neqs;
	uint32_t pint;
	int ret, len;
	
	/* default values */
	neqs = 1;
	pint = 0;
	if (!src) {
		ivp->neqs = neqs;
		ivp->pint = pint;
		if (ivp->grid) {
			free(ivp->grid);
			ivp->grid = 0;
		}
		return 0;
	}
	/* direct distinct conversions */
	len = 0;
	if ((ret = src->_vptr->convert(src, 'i', &neqs)) > 0) {
		if (neqs < 1) {
			return MPT_ERROR(BadValue);
		}
		if ((len = src->_vptr->convert(src, MPT_type_toVector('d'), &grid)) > 0) {
			size_t part = grid.iov_len / sizeof(double);
			if (part < 2 || part >= UINT32_MAX) {
				return MPT_ERROR(BadValue);
			}
			pint = part - 1;
		}
		if (!len || pint < 1) {
			ivp->neqs = neqs;
			ivp->pint = 0;
			if (ivp->grid) {
				free(ivp->grid);
			}
			return 0;
		}
		else if (len > 0) {
			double *tmp;
			size_t size = (pint + 1) * sizeof(*tmp);
			if (!(tmp = malloc(size))) {
				return MPT_ERROR(BadOperation);
			}
			if (ivp->grid) {
				free(ivp->grid);
			}
			if (grid.iov_base) {
				ivp->grid = memcpy(tmp, grid.iov_base, size);
			} else {
				ivp->grid = memset(tmp, 0, size);
			}
			return 0;
		}
		len = 0;
	}
	/* get values from iterator */
	if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) >= 0) {
		const MPT_STRUCT(value) *val;
		if (!ret || !it) {
			ivp->neqs = neqs;
			ivp->pint = pint;
			return 0;
		}
		if (!(val = it->_vptr->value(it))
		 || !val->type
		 || !val->ptr) {
			return MPT_ERROR(MissingData);
		}
		if (MPT_type_isConvertable(val->type)) {
			MPT_INTERFACE(convertable) *conv = *((void * const *) val->ptr);
			
			if (!conv) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = conv->_vptr->convert(conv, 'i', &neqs)) < 0) {
				return ret;
			}
		}
		else if (val->type == 'i') {
			neqs = *((const int *) val->ptr);
		}
		else {
			ivp->neqs = neqs;
			ivp->pint = pint;
			return 0;
		}
		if (neqs < 1) {
			return MPT_ERROR(BadValue);
		}
		if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		len = 1;
		if (!(val = it->_vptr->value(it))
		 || !val->type
		 || !val->ptr) {
			return len;
		}
		/* PDE without grid data */
		if (val->type == MPT_type_toVector('d')) {
			const struct iovec *vec = val->ptr;
			grid = *vec;
			ret = 1;
		}
		else if (MPT_type_isConvertable(val->type)) {
			MPT_INTERFACE(convertable) *conv = *((void * const *) val->ptr);
			
			if (!conv) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = conv->_vptr->convert(conv, MPT_type_toVector('d'), &grid)) < 0) {
				if ((ret = conv->_vptr->convert(conv, 'u', &pint)) < 0) {
					int32_t tmp;
					if ((ret = conv->_vptr->convert(conv, 'i', &tmp)) < 0) {
						return ret;
					}
					if (ret > 0) {
						if (tmp < 0) {
							return MPT_ERROR(BadValue);
						}
						pint = tmp;
					}
				}
			}
			if (!ret) {
				pint = 0;
			}
			else if ((ret = it->_vptr->advance(it)) < 0) {
				return ret;
			}
			else {
				len = 2;
			}
			if (ivp->grid) {
				free(ivp->grid);
				ivp->grid = 0;
			}
		}
		/* copy grid data for PDE */
		if (ret) {
			double *ptr;
			size_t part = grid.iov_len / sizeof(double);
			if (part < 2 || part > UINT32_MAX) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = it->_vptr->advance(it)) < 0) {
				return ret;
			}
			pint = part - 1;
			part *= sizeof(double);
			if (!(ptr = malloc(part))) {
				return MPT_ERROR(BadOperation);
			}
			if (grid.iov_base) {
				memcpy(ptr, grid.iov_base, part);
			} else {
				memset(ptr, 0, part);
			}
			if (ivp->grid) {
				free(ivp->grid);
			}
			ivp->grid = ptr;
			len = 2;
		}
		/* no grid data equals ODE/DAE mode */
		else if (ivp->grid) {
			free(ivp->grid);
			ivp->grid = 0;
		}
	}
	ivp->neqs = neqs;
	ivp->pint = pint;
	return len;
}
