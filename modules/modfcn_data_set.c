/*!
 * MPT solver module helper function
 *   assign segments or elements on solver data
 */

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>

#include "meta.h"

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(data_set)(MPT_SOLVER_MODULE_DATA_TYPE *dest, int32_t elem, long parts, MPT_INTERFACE(iterator) *it)
{
	struct iovec vec;
	uint32_t len;
	int ret;
	
	if (elem < 1 || parts < 0) {
		return MPT_ERROR(BadArgument);
	}
	/* reset target data */
	if (!it) {
		size_t total;
		if (!parts) {
			total = elem;
		} else {
			total = elem * parts;
		}
		memset(dest, 0, total * sizeof(*dest));
		return 0;
	}
	if (!parts) {
		/* get complete data segment */
		if ((ret = it->_vptr->get(it, MPT_type_vector(MPT_SOLVER_MODULE_DATA_ID), &vec)) >= 0) {
			if (!ret) {
				parts = 0;
			}
			else if ((ret = it->_vptr->advance(it)) < 0) {
				return ret;
			}
			else {
				if ((parts = vec.iov_len / sizeof(MPT_SOLVER_MODULE_DATA_TYPE)) > elem) {
					parts = elem;
				}
				ret = 1;
			}
			if (parts) {
				if (vec.iov_base) {
					memcpy(dest, vec.iov_base, parts * sizeof(MPT_SOLVER_MODULE_DATA_TYPE));
				} else {
					parts = 0;
				}
			}
			for ( ; parts < elem; ++parts) {
				dest[parts] = 0;
			}
			return ret;
		}
		len = 0;
		/* get single elements */
		while (parts < elem) {
			if ((ret = it->_vptr->get(it, MPT_SOLVER_MODULE_DATA_ID, dest + parts)) <= 0) {
				break;
			}
			++parts;
			if ((ret = it->_vptr->advance(it)) < 0) {
				break;
			}
			++len;
			if (!ret) {
				break;
			}
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
		if ((ret = it->_vptr->get(it, MPT_type_vector(MPT_SOLVER_MODULE_DATA_ID), &vec)) < 0) {
			/* read constant profile values */
			for (curr = 0; curr < elem; ++curr) {
				if ((ret = it->_vptr->get(it, MPT_SOLVER_MODULE_DATA_ID, dest + curr)) <= 0
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
		else if ((curr = vec.iov_len / sizeof(MPT_SOLVER_MODULE_DATA_TYPE))) {
			if (curr > elem) {
				curr = elem;
			}
			memcpy(dest, vec.iov_base, curr * sizeof(MPT_SOLVER_MODULE_DATA_TYPE));
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
		ret = len;
	}
	/* repeat last profile segment */
	if (len) {
		while (len++ < parts) {
			memcpy(dest, dest - elem, elem * sizeof(MPT_SOLVER_MODULE_DATA_TYPE));
			dest += elem;
		}
	}
	return ret;
}
