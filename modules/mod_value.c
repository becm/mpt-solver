/*!
 * MPT solver module helper function
 *   value representation for common data types
 */

#include <string.h>

#include "types.h"

#include "../solver.h"

extern int mpt_solver_module_value_rvec(MPT_STRUCT(property) *pr, long pos, const struct iovec *vec)
{
	double *ptr;
	size_t len;
	
	if (!pos) {
		MPT_value_set(&pr->val, MPT_type_toVector('d'), vec);
		if (vec && (sizeof(pr->_buf) >= sizeof(*vec))) {
			pr->val.ptr = memcpy(&pr->_buf, vec, sizeof(*vec));
			return vec->iov_len ? 3 : 2;
		}
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
		MPT_value_set(&pr->val, 'd', 0);
		return 0;
	}
	ptr += pos;
	mpt_solver_module_value_double(pr, ptr);
	return *ptr ? 1 : 0;
}
extern int mpt_solver_module_value_ivec(MPT_STRUCT(property) *pr, long pos, const struct iovec *vec)
{
	int *ptr;
	size_t len;
	
	if (!pos) {
		MPT_value_set(&pr->val, MPT_type_toVector('i'), vec);
		if (vec && (sizeof(pr->_buf) >= sizeof(*vec))) {
			pr->val.ptr = memcpy(&pr->_buf, vec, sizeof(*vec));
			return vec->iov_len ? 3 : 2;
		}
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
		MPT_value_set(&pr->val, 'i', 0);
		return 0;
	}
	ptr += pos;
	mpt_solver_module_value_int(pr, ptr);
	return *ptr ? 1 : 0;
}

extern int mpt_solver_module_value_set(MPT_STRUCT(property) *pr, int type, const void *ptr, size_t len)
{
	if (len > sizeof(pr->_buf)) {
		return MPT_ERROR(MissingBuffer);
	}
	if (ptr) {
		ptr = memcpy(pr->_buf, ptr, len);
	}
	else {
		ptr = memset(pr->_buf, 0, len);
	}
	MPT_value_set(&pr->val, type, ptr);
	return sizeof(pr->_buf) - len;
}
extern int mpt_solver_module_value_int(MPT_STRUCT(property) *pr, const int *ptr)
{
	int len = sizeof(*ptr);
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
	if (mpt_solver_module_value_set(pr, type, ptr, len) < 0) {
		MPT_value_set(&pr->val, type, ptr);
	}
	return ptr && *ptr != 0;
}
extern int mpt_solver_module_value_long(MPT_STRUCT(property) *pr, const long *ptr)
{
	int len = sizeof(*ptr);
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
	if (mpt_solver_module_value_set(pr, type, ptr, len) < 0) {
		MPT_value_set(&pr->val, type, ptr);
	}
	return ptr && *ptr != 0;
}

extern int mpt_solver_module_value_double(MPT_STRUCT(property) *pr, const double *ptr)
{
	if (mpt_solver_module_value_set(pr, 'd', ptr, sizeof(*ptr)) < 0) {
		MPT_value_set(&pr->val, 'd', ptr);
	}
	return ptr && *ptr != 0.0;
}

extern int mpt_solver_module_value_string(MPT_STRUCT(property) *pr, const char *ptr)
{
	int ret = ptr ? 1 : 0;
	size_t len;
	/* save full data in buffer area */
	if (ptr && (((len = strlen(ptr)) + sizeof(ptr)) < sizeof(pr->_buf))) {
		ptr = memcpy(pr->_buf + sizeof(ptr), ptr, len + 1);
		ret = 2;
	}
	*((const char **) pr->_buf) = ptr;
	MPT_value_set(&pr->val, 's', pr->_buf);
	return ret;
}
