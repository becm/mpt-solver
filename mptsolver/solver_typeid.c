/*!
 * MPT solver library
 *   register/get solver metatype code
 */

#include "message.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get solver id
 * 
 * Get unique registered dynamic metatype ID for solver interface.
 * 
 * \return solver interface ID
 */
extern int mpt_solver_typeid(void)
{
	static int id = 0;
	
	if (!id && (id = mpt_type_meta_new("solver")) < 0) {
		id = mpt_type_meta_new(0);
	}
	return id;
}
