/*!
 * push IVP history header
 */

#include "message.h"
#include "output.h"

#include "solver.h"

/*!
 * \ingroup mptSolver
 * \brief close solver output
 * 
 * Remove all references held by solver output.
 * 
 * \param out  solver output descriptor
 */
extern void mpt_solver_output_close(MPT_STRUCT(solver_output) *so)
{
	MPT_INTERFACE(unrefable) *ref;
	
	if ((ref = (void*) so->_data)) {
		ref->_vptr->unref(ref);
		so->_data = 0;
	}
	if ((ref = (void*) so->_graphic)) {
		ref->_vptr->unref(ref);
		so->_graphic = 0;
	}
	if ((ref = (void*) so->_info)) {
		ref->_vptr->unref(ref);
		so->_info = 0;
	}
	if ((ref = (void*) so->_pass._buf)) {
		ref->_vptr->unref(ref);
		so->_pass._buf = 0;
	}
}
