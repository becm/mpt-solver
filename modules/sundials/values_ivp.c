/*!
 * Sundials N_Vector creation
 */

#include <limits.h>
#include <string.h>

#include <sys/uio.h>

#include <sundials/sundials_nvector.h>

#include "array.h"

#include "sundials.h"

extern int sundials_values_ivp(N_Vector *v, const MPT_SOLVER_STRUCT(ivppar) *ivp, MPT_INTERFACE(metatype) *src)
{
	MPT_STRUCT(array) *arr;
	struct iovec vec;
	N_Vector next;
	double val, *dest;
	long required;
	
	if (ivp->neqs < 1 || (required = ivp->pint) < 0
	    || required++ == LONG_MAX || LONG_MAX/ivp->neqs < required) {
		return MPT_ERROR(BadArgument);
	}
	required *= ivp->neqs;
	
	if (!src) {
		if (!(next = sundials_nvector_new(required))) {
			return MPT_ERROR(BadOperation);
		}
		return 0;
	}
	if (src->_vptr->conv(src, MPT_value_toArray('d') | MPT_ENUM(ValueConsume), &arr) >= 0) {
		MPT_STRUCT(buffer) *buf;
		if (!arr || !(buf = arr->_buf) || (long) (buf->used/sizeof(double)) != required) {
			return MPT_ERROR(BadArgument);
		}
		if (!(next = sundials_nvector_new(required))) {
			return MPT_ERROR(BadOperation);
		}
		if (*v) {
			N_VDestroy(*v);
		}
		*v = next;
		dest = memcpy(N_VGetArrayPointer(next), buf+1, required * sizeof(double));
		
		return 1;
	}
	if (src->_vptr->conv(src, MPT_value_toVector('d') | MPT_ENUM(ValueConsume), &vec) >= 0) {
		if ((long) (vec.iov_len/sizeof(double)) != required) {
			return MPT_ERROR(BadArgument);
		}
		if (!(next = sundials_nvector_new(required))) {
			return MPT_ERROR(BadOperation);
		}
		if (*v) {
			N_VDestroy(*v);
		}
		*v = next;
		dest = N_VGetArrayPointer(next);
		if (vec.iov_base) {
			memcpy(dest, vec.iov_base, required * sizeof(double));
		} else {
			memset(dest, 0, required * sizeof(double));
		}
		return 1;
	}
	if (src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val) < 0) {
		return MPT_ERROR(BadType);
	}
	if (!(next = sundials_nvector_new(required))) {
		return MPT_ERROR(BadOperation);
	}
	dest = N_VGetArrayPointer(next);
	
	*dest++ = val;
	while (--required) {
		if (src->_vptr->conv(src, 'd' | MPT_ENUM(ValueConsume), &val) < 0) {
			N_VDestroy(next);
			return MPT_ERROR(BadType);
		}
		*dest++ = val;
	}
	return 2;
}
