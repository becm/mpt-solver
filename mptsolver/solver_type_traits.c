/*!
 * MPT solver library
 *   register/get solver metatype code
 */

#include "message.h"
#include "output.h"
#include "types.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get solver id
 * 
 * Get unique registered dynamic metatype ID for solver interface.
 * 
 * \return solver interface ID
 */
extern const MPT_STRUCT(named_traits) *mpt_solver_type_traits(void)
{
	static const MPT_STRUCT(named_traits) *traits = 0;
	
	if (!traits && !(traits = mpt_type_metatype_add("solver"))) {
		traits = mpt_type_metatype_add(0);
	}
	return traits;
}
