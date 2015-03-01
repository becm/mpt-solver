/*!
 * wrapper for CVode right side.
 */

#include <errno.h>

#include <cvode/cvode.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode user function wrapper
 * 
 * Wrapper to call mpt::ivpfcn entries from CVode solver.
 * For parameter description see Sundials documatation.
 * 
 * \return result of user function
 */
extern int sundials_cvode_fcn(realtype t, N_Vector y, N_Vector f, void *data)
{
	MPT_SOLVER_STRUCT(cvode) *cv;
	const MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	
	if (!(cv = data) || !(fcn = cv->ufcn) || !fcn->fcn) {
		errno = EFAULT; return CV_MEM_NULL;
	}
	return fcn->fcn(fcn->param, &t, N_VGetArrayPointer(y), N_VGetArrayPointer(f));
}



