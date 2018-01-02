/*!
 * MPT solver module helper function
 *   prepare vecpar data
 */

#include "../solver.h"

extern int mpt_solver_module_value_rvec(MPT_STRUCT(value) *val, int pos, const struct iovec *vec)
{
	double *ptr;
	
	if (!vec
	    || pos < 1
	    || !(ptr = vec->iov_base)
	    || vec->iov_len < pos * sizeof(double)) {
		static const uint8_t fmt[] = "d";
		val->fmt = fmt;
		val->ptr = 0;
		return 0;
	}
	ptr += pos - 1;
	mpt_solver_module_value_double(val, ptr);
	return *ptr ? 1 : 0;
}
extern void mpt_solver_module_value_double(MPT_STRUCT(value) *val, const double *ptr)
{
	static const uint8_t fmt[] = "d";
	val->fmt = fmt;
	val->ptr = ptr;
}
extern int mpt_solver_module_value_ivec(MPT_STRUCT(value) *val, int pos, const struct iovec *vec)
{
	int *ptr;
	
	if (!vec
	    || pos < 1
	    || !(ptr = vec->iov_base)
	    || vec->iov_len < pos * sizeof(int)) {
		static const uint8_t fmt[] = "i";
		val->fmt = fmt;
		val->ptr = 0;
		return 0;
	}
	ptr += pos - 1;
	mpt_solver_module_value_int(val, ptr);
	return *ptr ? 1 : 0;
}
extern void mpt_solver_module_value_int(MPT_STRUCT(value) *val, const int *ptr)
{
	static const uint8_t fmt[] = "i";
	val->fmt = fmt;
	val->ptr = ptr;
}
