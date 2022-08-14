/*!
 * MPT solver library
 *   register/get solver metatype code
 */

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get solver traits
 * 
 * Get or register unique dynamic traits for solver metatype.
 * 
 * \return solver metatype taits
 */
extern const MPT_STRUCT(named_traits) *mpt_solver_type_traits(void)
{
	static const MPT_STRUCT(named_traits) *traits = 0;
	
	if (!traits && !(traits = mpt_type_metatype_add("solver"))) {
		traits = mpt_type_metatype_add(0);
	}
	return traits;
}
