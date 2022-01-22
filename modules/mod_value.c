/*!
 * MPT solver module helper function
 *   value representation for common data types
 */

#include <string.h>

#include "types.h"

#include "../solver.h"

extern int mpt_solver_module_value_rvec(MPT_STRUCT(value) *val, long pos, const struct iovec *vec)
{
	double *ptr;
	size_t len;
	
	if (!pos) {
		val->domain = 0;
		val->type = MPT_type_toVector('d');
		if (vec && val->_bufsize >= sizeof(*vec)) {
			val->ptr = memcpy(&val->_buf, vec, sizeof(*vec));
			return vec->iov_len ? 3 : 2;
		}
		val->ptr = vec;
		return (vec && vec->iov_len) ? 1 : 0;
	}
	if (!vec) {
		return MPT_ERROR(MissingData);
	}
	if (pos < 0) {
		len  = (-pos) * sizeof(double);
		pos += vec->iov_len / sizeof(double);
	} else {
		len  = pos-- * sizeof(double);
	}
	
	if (!(ptr = vec->iov_base)
	  || vec->iov_len < len) {
		val->domain = 0;
		val->type = 'd';
		val->ptr = 0;
		return 0;
	}
	ptr += pos;
	mpt_solver_module_value_double(val, ptr);
	return *ptr ? 1 : 0;
}
extern int mpt_solver_module_value_ivec(MPT_STRUCT(value) *val, long pos, const struct iovec *vec)
{
	int *ptr;
	size_t len;
	
	if (!pos) {
		val->domain = 0;
		val->type = MPT_type_toVector('i');
		if (vec && val->_bufsize >= sizeof(*vec)) {
			val->ptr = memcpy(&val->_buf, vec, sizeof(*vec));
			return vec->iov_len ? 3 : 2;
		}
		val->ptr = vec;
		return (vec && vec->iov_len) ? 1 : 0;
	}
	if (!vec) {
		return MPT_ERROR(MissingData);
	}
	if (pos < 0) {
		len  = (-pos) * sizeof(int);
		pos += vec->iov_len / sizeof(int);
	} else {
		len  = pos-- * sizeof(int);
	}
	
	if (!(ptr = vec->iov_base)
	  || vec->iov_len < len) {
		val->domain = 0;
		val->type = 'i';
		val->ptr = 0;
		return 0;
	}
	ptr += pos;
	mpt_solver_module_value_int(val, ptr);
	return *ptr ? 1 : 0;
}

extern void mpt_solver_module_value_double(MPT_STRUCT(value) *val, const double *ptr)
{
	val->domain = 0;
	val->type = 'd';
	if (ptr && val->_bufsize >= sizeof(*ptr)) {
		val->ptr = memcpy(val->_buf, ptr, sizeof(*ptr));
	} else {
		val->ptr = ptr;
	}
}

extern void mpt_solver_module_value_signed(MPT_STRUCT(value) *val, const void *ptr, size_t len)
{
	int type;
	switch (len) {
		case sizeof(int8_t) : type = 'b'; break;
		case sizeof(int16_t): type = 'n'; break;
		case sizeof(int32_t): type = 'i'; break;
		case sizeof(int64_t): type = 'x'; break;
		default:
			ptr = 0;
			type = 0;
	}
	val->domain = 0;
	val->type = type;
	if (ptr && len <= val->_bufsize) {
		val->ptr = memcpy(val->_buf, ptr, len);
	} else {
		val->ptr = ptr;
	}
}
extern void mpt_solver_module_value_int(MPT_STRUCT(value) *val, const int *ptr)
{
	mpt_solver_module_value_signed(val, ptr, sizeof(*ptr));
}
extern void mpt_solver_module_value_long(MPT_STRUCT(value) *val, const long *ptr)
{
	mpt_solver_module_value_signed(val, ptr, sizeof(*ptr));
}

extern void mpt_solver_module_value_string(MPT_STRUCT(value) *val, const char *ptr)
{
	val->domain = 0;
	val->type = 's';
	if (val->_bufsize >= sizeof(ptr)) {
		size_t len;
		/* save full data in value */
		if (ptr && (((len = strlen(ptr)) + sizeof(ptr)) < val->_bufsize)) {
			ptr = memcpy(val->_buf + sizeof(ptr), ptr, len + 1);
		}
		*((const char **) val->_buf) = ptr;
		val->ptr = val->_buf;
	} else {
		val->ptr = 0;
	}
}
