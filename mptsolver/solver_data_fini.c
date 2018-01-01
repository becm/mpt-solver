/*!
 * initialize/finalize solver data descriptor.
 */

#include <string.h>
#include <errno.h>

#include "array.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief clear solver data
 * 
 * Reset solver data to default values.
 * 
 * \param sd  solver data pointer
 */
extern void mpt_solver_data_clear(MPT_STRUCT(solver_data) *md)
{
	MPT_STRUCT(buffer) *buf;
	
	if ((buf = md->param._buf)) {
		buf->_used = 0;
	}
	if ((buf = md->val._buf)) {
		buf->_used = 0;
	}
	md->npar = 0;
	md->nval = 0;
}
/*!
 * \ingroup mptSolver
 * \brief terminate solver data
 * 
 * Clear solver data resources.
 * 
 * \param sd  solver data pointer
 */
extern void mpt_solver_data_fini(MPT_STRUCT(solver_data) *md)
{
	mpt_array_clone(&md->param, 0);
	mpt_array_clone(&md->val, 0);
}

