/*!
 * set double values on vector
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_ivpset(MPT_SOLVER_IVP_STRUCT(parameters) *ivp, const MPT_INTERFACE(metatype) *src)
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
	if ((ret = src->_vptr->conv(src, 'i', &neqs)) > 0) {
		if (neqs < 1) {
			return MPT_ERROR(BadValue);
		}
		if ((len = src->_vptr->conv(src, MPT_value_toVector('d'), &grid)) > 0) {
			size_t part = grid.iov_len / sizeof(double);
			if (part < 2 || part >= UINT32_MAX) {
				return MPT_ERROR(BadValue);
			}
			pint = part - 1;
		}
		if (!len) {
			ivp->neqs = neqs;
			ivp->pint = pint;
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
	}
	/* get values from iterator */
	if ((ret = src->_vptr->conv(src, MPT_ENUM(TypeIterator), &it)) >= 0) {
		if (!ret || !it) {
			ivp->neqs = neqs;
			ivp->pint = pint;
			return 0;
		}
		if ((ret = it->_vptr->get(it, 'i', &neqs)) < 0) {
			return ret;
		}
		else if (!ret) {
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
		/* PDE without grid data */
		if ((ret = it->_vptr->get(it, MPT_value_toVector('d'), &grid)) < 0) {
			if ((ret = it->_vptr->get(it, 'u', &pint)) < 0) {
				return ret;
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
		else if (ret) {
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
			if (!(ptr = malloc(grid.iov_len))) {
				return MPT_ERROR(BadOperation);
			}
			if (grid.iov_base) {
				memcpy(ptr, grid.iov_base, grid.iov_len);
			} else {
				memset(ptr, 0, grid.iov_len);
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
