/*!
 * Sundials N_Vector creation
 */

#include <limits.h>
#include <string.h>

#include <sys/uio.h>

#include <sundials/sundials_nvector.h>

#include "array.h"

#include "meta.h"

#include "sundials.h"

extern int sundials_vector_set(N_Vector *v, long len, MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(array) *arr;
	struct iovec vec;
	N_Vector next;
	realtype val, *dest;
	size_t size;
	long pos;
	int res;
	
	if (len <= 0) {
		return MPT_ERROR(BadArgument);
	}
	size = len * sizeof(*dest);
	
	if (!src) {
		if (!(next = sundials_nvector_new(len))) {
			return MPT_ERROR(BadOperation);
		}
		if (*v) {
			N_VDestroy(*v);
		}
		*v = next;
		memset(N_VGetArrayPointer(next), 0, size);
		return 0;
	}
	if ((res = src->_vptr->conv(src, MPT_value_toVector(MPT_SOLVER_ENUM(SundialsRealtype)) | MPT_ENUM(ValueConsume), &vec)) > 0) {
		if (vec.iov_len != size) {
			return MPT_ERROR(BadArgument);
		}
		if (!(next = sundials_nvector_new(len))) {
			return MPT_ERROR(BadOperation);
		}
		if (*v) {
			N_VDestroy(*v);
		}
		*v = next;
		dest = N_VGetArrayPointer(next);
		if (vec.iov_base) {
			memcpy(dest, vec.iov_base, size);
		} else {
			memset(dest, 0, size);
		}
		return 1;
	}
	if (res && (res = src->_vptr->conv(src, MPT_value_toArray(MPT_SOLVER_ENUM(SundialsRealtype)) | MPT_ENUM(ValueConsume), &arr)) > 0) {
		MPT_STRUCT(buffer) *buf;
		if (!arr || !(buf = arr->_buf) || buf->used != size) {
			return MPT_ERROR(BadArgument);
		}
		if (!(next = sundials_nvector_new(len))) {
			return MPT_ERROR(BadOperation);
		}
		if (*v) {
			N_VDestroy(*v);
		}
		*v = next;
		dest = memcpy(N_VGetArrayPointer(next), buf+1, size);
		
		return 1;
	}
	if (res && (res = src->_vptr->conv(src, MPT_SOLVER_ENUM(SundialsRealtype) | MPT_ENUM(ValueConsume), &val)) < 0) {
		return MPT_ERROR(BadType);
	}
	if (!(next = sundials_nvector_new(len))) {
		return MPT_ERROR(BadOperation);
	}
	dest = N_VGetArrayPointer(next);
	
	if (!res) {
		memset(dest, 0, size);
		if (*v) {
			N_VDestroy(*v);
		}
		*v = next;
		return 0;
	}
	*dest = val;
	pos = 1;
	while (pos < len) {
		res = src->_vptr->conv(src, MPT_SOLVER_ENUM(SundialsRealtype) | MPT_ENUM(ValueConsume), &val);
		if (res < 0) {
			N_VDestroy(next);
			return MPT_ERROR(BadType);
		}
		if (!res) {
			break;
		}
		dest[pos++] = val;
	}
	for ( ; pos < len; ++pos) dest[pos] = 0;
	
	if (*v) {
		N_VDestroy(*v);
	}
	*v = next;
	
	return pos;
}
