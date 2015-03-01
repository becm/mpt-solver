
#include "solver.h"

#include "client.h"

/*!
 * \ingroup mptClient
 * \brief solver type check
 * 
 * Test if supplied solver con handle
 * requested problem type(s)
 * 
 * \param solv pointer to solver interface
 * \param type problem type mask
 */
extern int mpt_solver_check(MPT_SOLVER_INTERFACE *solv, int type)
{
	MPT_STRUCT(property) prop;
	int mode;
	
	prop.name = "";
	prop.desc = 0;
	if ((mode = solv->_vptr->_mt.property((void*) solv, &prop, 0)) < 0) {
		return -1;
	}
	/* any supported type is accepted */
	if (type < 0) {
		type = -type;
		if (!(type & mode)) {
			return -2;
		}
	}
	/* all types must be accepted */
	else if ((type & mode) != type) {
		return -3;
	}
	return mode;
}
