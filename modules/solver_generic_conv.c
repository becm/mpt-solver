/*!
 * MPT solver module helper function
 *   convert from generic solver with object interface
 */

#include "../solver.h"

extern int mpt_solver_generic_conv(const MPT_SOLVER(generic) *gen, int type, void *ptr)
{
	if (!type) {
		static const char fmt[] = { MPT_ENUM(TypeObject), 0 };
		if (ptr) *((const char **) ptr) = fmt;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = &gen->_obj;
		return MPT_ENUM(TypeMeta);
	}
	if (type == MPT_ENUM(TypeMeta)) {
		if (ptr) *((const void **) ptr) = &gen->_sol;
		return MPT_ENUM(TypeObject);
	}
	return MPT_ERROR(BadType);
}
