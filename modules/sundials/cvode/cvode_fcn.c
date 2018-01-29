/*!
 * wrapper for CVode right side.
 */

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
extern int mpt_sundials_cvode_fcn(realtype t, N_Vector y, N_Vector f, const MPT_SOLVER_STRUCT(cvode) *cv)
{
	const MPT_IVP_STRUCT(pdefcn) *pde;
	double *fd, *yd;
	if (!cv || !(pde = (void *) cv->ufcn) || !pde->fcn) {
		return CV_MEM_NULL;
	}
	yd = N_VGetArrayPointer(y);
	fd = N_VGetArrayPointer(f);
	
	return pde->fcn(pde->par, t, yd, fd, &cv->ivp);
}
