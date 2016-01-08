/*!
 * create output values for BACOL descriptor
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_values(MPT_SOLVER_STRUCT(bacol) *bac, double *xout, int deriv, double *yout)
{
	int wlen, kcol, dim;
	
	if (bac->ivp.neqs < 1
	    || (dim = bac->out.len + 1) < 2
	    || !bac->mflag.noinit) {
		return MPT_ERROR(BadArgument);
	}
	/* use internal grid output */
	if (!xout) {
		if (!(xout = realloc(bac->out.x, dim * sizeof(*xout)))) {
			return MPT_ERROR(BadOperation);
		}
		if (!bac->out.x) {
			double dx = 1.0 / (dim - 1);
			int i;
			for (i = 0; i < dim; ++i) xout[i] = i * dx;
		}
		bac->out.x = xout;
	}
	/* use internal state output */
	if (!yout) {
		size_t l = bac->ivp.neqs * dim * sizeof(*yout);
		if (!(yout = realloc(bac->out.y, l))) {
			return MPT_ERROR(BadOperation);
		}
		if (!bac->out.y && deriv < 0) {
			memset(yout, 0, l);
		}
		bac->out.y = yout;
	}
	/* adapt grid points to integrator intervals */
	if (bac->grid
	    && (dim = bac->grid(bac, dim, xout)) < 0) {
		return dim;
	}
	kcol = bac->kcol;
	
	/* check output work data availability */
	wlen = (kcol * MPT_BACOL_NCONTI) * (1 + deriv);
	wlen += kcol * (bac->nint + 1) + 2 * MPT_BACOL_NCONTI;
	
	if (!mpt_vecpar_alloc(&bac->out.wrk, wlen, sizeof(double))) {
		return MPT_ERROR(BadOperation);
	}
	/* generate y-values for grid */
	if (deriv >= 0) {
		values_(&kcol, xout, &bac->nint, bac->x, &bac->ivp.neqs, &dim,
		        &deriv, yout, bac->y, bac->out.wrk.iov_base);
	}
	return dim;
}


