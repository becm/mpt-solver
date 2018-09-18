/*!
 * MPT solver module helper function
 *   value representation for common data types
 */

#include "../solver.h"

extern int mpt_solver_module_value_rvec(MPT_STRUCT(value) *val, long pos, const struct iovec *vec)
{
	double *ptr;
	size_t len;
	
	if (!pos) {
		static const uint8_t fmt[] = "D";
		val->fmt = fmt;
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
		static const uint8_t fmt[] = "d";
		val->fmt = fmt;
		val->ptr = 0;
		return 0;
	}
	ptr += pos;
	mpt_solver_module_value_double(val, ptr);
	return *ptr ? 1 : 0;
}
extern void mpt_solver_module_value_double(MPT_STRUCT(value) *val, const double *ptr)
{
	static const uint8_t fmt[] = "d";
	val->fmt = fmt;
	val->ptr = ptr;
}
extern int mpt_solver_module_value_ivec(MPT_STRUCT(value) *val, long pos, const struct iovec *vec)
{
	int *ptr;
	size_t len;
	
	if (!pos) {
		static const uint8_t fmt[] = "I";
		val->fmt = fmt;
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
		static const uint8_t fmt[] = "i";
		val->fmt = fmt;
		val->ptr = 0;
		return 0;
	}
	ptr += pos;
	mpt_solver_module_value_int(val, ptr);
	return *ptr ? 1 : 0;
}
extern void mpt_solver_module_value_int(MPT_STRUCT(value) *val, const int *ptr)
{
	static const uint8_t fmt[] = "i";
	val->fmt = fmt;
	val->ptr = ptr;
}
