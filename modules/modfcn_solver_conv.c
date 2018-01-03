/*!
 * MPT module helper function
 *   convert from generic "metatype with object" interface
 */

#include "solver_modfcn.h"

/*!
 * \ingroup mptSolver
 * \brief generic converion
 * 
 * Convert to metatype or object interface.
 * 
 * \param sol   solver interface
 * \param obj   object interface
 * \param type  target type
 * \param ptr   target address
 * 
 * \return conversion result
 */
extern int MPT_SOLVER_MODULE_FCN(solver_conv)(const MPT_SOLVER(interface) *sol, const MPT_INTERFACE(object) *obj, int type, void *ptr)
{
	if (!type) {
		static const uint8_t fmt[] = { MPT_ENUM(TypeSolver), MPT_ENUM(TypeObject), 0 };
		if (ptr) {
			*((const uint8_t **) ptr) = obj ? fmt : fmt + 1;
			return 0;
		}
		return MPT_ENUM(TypeSolver);
	}
	if (type == MPT_ENUM(TypeSolver)) {
		if (ptr) *((const void **) ptr) = sol;
		return obj ? MPT_ENUM(TypeObject) : 0;
	}
	if (type == MPT_ENUM(TypeObject)) {
		if (ptr) *((const void **) ptr) = obj;
		return MPT_ENUM(TypeSolver);
	}
	return MPT_ERROR(BadType);
}
