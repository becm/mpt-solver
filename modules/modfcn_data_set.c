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
	const MPT_STRUCT(value) *val;
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
	/* get complete data segment */
	if (!(val = it->_vptr->value(it))
	 || !val->_addr) {
		return MPT_ERROR(MissingData);
	}
	if (!parts) {
		if (val->_type == MPT_type_toVector(MPT_SOLVER_MODULE_DATA_ID)) {
			vec = *((const struct iovec *) val->_addr);
			
			parts = vec.iov_len / sizeof(MPT_SOLVER_MODULE_DATA_TYPE);
			if (parts > elem) {
				parts = elem;
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
			if ((ret = it->_vptr->advance(it)) < 0) {
				return 0;
			}
			return 1;
		}
		len = 0;
		/* get single elements */
		while (parts < elem) {
			MPT_INTERFACE(convertable) *conv = *((void * const *) val->_addr);
			
			if (val->_type == MPT_SOLVER_MODULE_DATA_ID) {
				dest[parts] = *((const MPT_SOLVER_MODULE_DATA_TYPE *) val->_addr);
			}
			else if (!MPT_type_isConvertable(val->_type)) {
				break;
			}
			else if ((ret = conv->_vptr->convert(conv, MPT_SOLVER_MODULE_DATA_ID, dest + parts)) < 0) {
				break;
			}
			++parts;
			if ((ret = it->_vptr->advance(it)) < 0) {
				break;
			}
			++len;
			if (!ret || !(val = it->_vptr->value(it)) || !val->_addr) {
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
		if (val->_type != MPT_type_toVector(MPT_SOLVER_MODULE_DATA_ID)) {
			/* read constant profile values */
			for (curr = 0; curr < elem; ++curr) {
				MPT_INTERFACE(convertable) *conv = *((void * const *) val->_addr);
				if (val->_type == MPT_SOLVER_MODULE_DATA_ID) {
					dest[curr] = *((const MPT_SOLVER_MODULE_DATA_TYPE *) val->_addr);
				}
				else if (!MPT_type_isConvertable(val->_type)) {
					break;
				}
				else if ((ret = conv->_vptr->convert(conv, MPT_SOLVER_MODULE_DATA_ID, dest + parts)) < 0) {
					break;
				}
				if ((ret = it->_vptr->advance(it)) <= 0) {
					break;
				}
				if (!(val = it->_vptr->value(it)) || !val->_addr) {
					break;
				}
			}
			dest += elem;
			ret += curr;
			len++;
			break;
		}
		/* copy profile segment data */
		vec = *((const struct iovec *) val->_addr);
		
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
		ret = ++len;
		/* no further data expected/available */
		if (it->_vptr->advance(it) <= 0
		 || !(val = it->_vptr->value(it))
		 || !val->_addr) {
			break;
		}
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
