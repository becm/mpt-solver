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
extern int sundials_cvode_fcn(realtype t, N_Vector y, N_Vector f, const MPT_SOLVER_STRUCT(cvode) *cv)
{
	const MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	double *fd, *yd;
	if (!cv || !(fcn = cv->ufcn) || !fcn->dae.fcn) {
		errno = EFAULT;
		return CV_MEM_NULL;
	}
	yd = N_VGetArrayPointer(y);
	fd = N_VGetArrayPointer(f);
	
	return ((const MPT_SOLVER_STRUCT(pdefcn) *) fcn)->fcn(fcn->dae.param, t, yd, fd, &cv->ivp, fcn->grid, fcn->rside);
}
