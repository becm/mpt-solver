/*!
 * push IVP history header
 */

#include "message.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief get log target
 * 
 * Get logging descriptor from solver output components.
 * 
 * \param out  solver output descriptor
 * 
 * \return log descriptor
 */
extern MPT_INTERFACE(logger) *mpt_solver_output_logger(const MPT_STRUCT(solver_output) *so)
{
	MPT_INTERFACE(metatype) *mt;
	MPT_INTERFACE(logger) *out = 0;
	
	if ((mt = so->_info)
	    && mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &out) >= 0
	    && out) {
		return out;
	}
	if ((mt = so->_data)
	    && mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &out) >= 0
	    && out) {
		return out;
	}
	if ((mt = so->_graphic)
	    && mt->_vptr->conv(mt, MPT_ENUM(TypeLogger), &out) >= 0
	    && out) {
		return out;
	}
	return 0;
}
