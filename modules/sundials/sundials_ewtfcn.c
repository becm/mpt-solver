/*!
 * Sundials error weight function.
 */

#include <math.h>

#include <sundials/sundials_nvector.h>

#include "sundials.h"

/*!
 * \ingroup mptSundials
 * \brief get error data
 * 
 * Calculate weighted error values for state vector
 * 
 * \param[in]  vy    solver state vector
 * \param[out] ve    error vector
 * \param      data  user data
 * 
 * \retval 0  no error
 */
extern int mpt_sundials_ewtfcn(N_Vector vy, N_Vector ve, void *data)
{
	struct {
		MPT_IVP_STRUCT(parameters) ivp;
		MPT_SOLVER_TYPE(dvecpar) rtol, atol;
	} *d = data;
	double *ewt, *y, *tol;
	long i, j, dim, neq;
	
	/* WARNING: untested for parallel vector implementation */
	y   = N_VGetArrayPointer(vy);
	ewt = N_VGetArrayPointer(ve);
	
	/* get equotation and grid point count */
	dim = d->ivp.pint + 1;
	neq = d->ivp.neqs;
	
	/* absolute tolerance is vector */
	if ((tol = d->atol._base)) {
		double *rtol;
		
		/* both tolerances are vectors */
		if ((rtol = d->rtol._base)) {
			for (j = 0; j < dim; j++, y += neq, ewt += neq) {
				for (i = 0; i < neq; i++) {
					ewt[i] = 1./(tol[i] + rtol[i] * fabs(y[i]));
				}
			}
		}
		/* relative tolerances is scalar */
		else  {
			double rt = d->rtol._d.val;
			for (j = 0; j < dim; j++, y += neq, ewt += neq) {
				for (i = 0; i < neq; i++) {
					ewt[i] = 1./(tol[i] + rt * fabs(y[i]));
				}
			}
		}
	}
	/* relative tolerance is vector */
	else if ((tol = d->rtol._base)) {
		double at = d->rtol._d.val;
		
		for (j = 0; j < dim; j++, y += neq, ewt += neq) {
			for (i = 0; i < neq ; i++) {
				ewt[i] = 1./(at + tol[i] * fabs(y[i]));
			}
		}
	}
	/* scalar tolerances */
	else {
		double rt, at;
		
		rt = d->rtol._d.val;
		at = d->atol._d.val;
		
		dim *= neq;
		
		for (i = 0; i < dim; i++) {
			ewt[i] = 1. / (at + rt * fabs(y[i]));
		}
	}
	
	return 0;
}
