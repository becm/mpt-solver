/*!
 * set double values on vector
 */

#include <limits.h>

#include <sys/uio.h>

#include "meta.h"

#include "../solver.h"

extern int mpt_solver_ivpset(MPT_SOLVER_IVP_STRUCT(parameters) *ivp, const MPT_INTERFACE(metatype) *src)
{
	MPT_INTERFACE(iterator) *it;
	struct iovec grid;
	int32_t neqs, pint;
	int ret, len;
	
	neqs = 1;
	pint = 0;
	if (!src) {
		ivp->neqs = neqs;
		ivp->pint = pint;
		return 0;
	}
	if ((ret = src->_vptr->conv(src, 'i', &neqs)) > 0) {
		if (neqs < 1) {
			return MPT_ERROR(BadValue);
		}
	}
	if ((len = src->_vptr->conv(src, MPT_value_toVector('d'), &grid)) > 0) {
		size_t part = grid.iov_len / sizeof(double);
		if (part < 2 || part > INT_MAX) {
			return MPT_ERROR(BadValue);
		}
		pint = part - 1;
	}
	if (ret > 0 || len > 0) {
		ivp->neqs = neqs;
		ivp->pint = pint;
		return 0;
	}
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
			return  MPT_ERROR(BadValue);
		}
		if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		if ((ret = it->_vptr->get(it, MPT_value_toVector('d'), &grid)) < 0) {
			if ((ret = it->_vptr->get(it, 'i', &pint)) < 0) {
				return ret;
			}
			if (ret && pint < 0) {
				return MPT_ERROR(BadValue);
			}
		}
		else if (ret) {
			size_t part = grid.iov_len / sizeof(double);
			if (part < 2 || part > INT_MAX) {
				return MPT_ERROR(BadValue);
			}
			pint = part - 1;
		}
		if (!ret) {
			ret = 1;
		}
		else if ((ret = it->_vptr->advance(it)) < 0) {
			return ret;
		}
		else {
			ret = 2;
		}
	}
	ivp->neqs = neqs;
	ivp->pint = pint;
	return ret;
}
