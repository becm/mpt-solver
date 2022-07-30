/*!
 * MPT solver module helper function
 *   set IVP equotation count and grid size/data
 */

#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "types.h"
#include "meta.h"

#include "../solver.h"

extern int mpt_solver_module_value_ivp(MPT_STRUCT(property) *pr, const MPT_IVP_STRUCT(parameters) *par)
{
	mpt_solver_module_value_int(pr, par ? &par->neqs : 0);
	return par && (par->neqs != 1 || par->pint) ? 1 : 0;
}

extern int mpt_solver_module_ivpset(MPT_IVP_STRUCT(parameters) *ivp, MPT_INTERFACE(convertable) *src)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec grid;
	size_t part;
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
	part = 0;
	it = 0;
	if ((ret = src->_vptr->convert(src, 'i', &neqs)) > 0) {
		if (neqs < 1) {
			return MPT_ERROR(BadValue);
		}
		if ((ret = src->_vptr->convert(src, MPT_type_toVector('d'), &grid)) > 0) {
			part = grid.iov_len / sizeof(double);
			if (part < 2 || part >= UINT32_MAX) {
				return MPT_ERROR(BadValue);
			}
		}
	}
	/* require iteratable source */
	else if ((ret = src->_vptr->convert(src, MPT_ENUM(TypeIteratorPtr), &it)) < 0) {
		return MPT_ERROR(BadType);
	}
	/* get values from active iterator */
	else if (ret && it) {
		const MPT_STRUCT(value) *val = it->_vptr->value(it);
		
		if (!val
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
			neqs = *((const int32_t *) val->ptr);
		}
		else {
			return MPT_ERROR(BadType);
		}
		if (neqs < 1) {
			return MPT_ERROR(BadValue);
		}
		if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		len = 1;
		if (!ret || !(val = it->_vptr->value(it))) {
			/* iterator has no further data */
		}
		else if (!val->type) {
			return MPT_ERROR(BadType);
		}
		else if (!val->ptr) {
			return MPT_ERROR(MissingData);
		}
		/* actual grid data */
		else if (val->type == MPT_type_toVector('d')) {
			const struct iovec *vec = val->ptr;
			grid = *vec;
			part = grid.iov_len / sizeof(double);
		}
		/* scalar interval values */
		else if (val->type == 'i') {
			pint = *((const int32_t *) val->ptr);
		}
		else if (val->type == 'u') {
			pint = *((const uint32_t *) val->ptr);
		}
		/* convertable value */
		else if (!MPT_type_isConvertable(val->type)) {
			MPT_INTERFACE(convertable) *conv = *((void * const *) val->ptr);
			
			if (!conv) {
				return MPT_ERROR(BadValue);
			}
			if ((ret = conv->_vptr->convert(conv, MPT_type_toVector('d'), &grid)) >= 0) {
				part = grid.iov_len / sizeof(double);
			}
			else if ((ret = conv->_vptr->convert(conv, 'u', &pint)) < 0) {
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
		else {
			return MPT_ERROR(BadType);
		}
	}
	/* copy grid data for PDE */
	if (part) {
		double *ptr;
		if (part < 2 || part > UINT32_MAX) {
			return MPT_ERROR(BadValue);
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
	}
	/* no grid data equals ODE/DAE mode */
	else if (ivp->grid) {
		free(ivp->grid);
		ivp->grid = 0;
	}
	if (it && (ret = it->_vptr->advance(it)) >= 0) {
		++len;
	}
	ivp->neqs = neqs;
	ivp->pint = pint;
	return len;
}
