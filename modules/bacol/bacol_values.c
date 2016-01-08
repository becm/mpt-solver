/*!
 * create output values for BACOL descriptor
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern int mpt_bacol_values(MPT_SOLVER_STRUCT(bacol) *bac, double *xout, int deriv, double *yout)
{
	int wlen, kcol, oint;
	
	if (bac->ivp.neqs < 1
	    || (oint = bac->out.nint) < 1
	    || !bac->mflag.noinit) {
		return MPT_ERROR(BadArgument);
	}
	/* conflict with existing derivation size */
	if (bac->out.nderiv >= 0) {
		if (deriv != bac->out.nderiv) {
			return MPT_ERROR(BadArgument);
		}
	}
	/* use internal grid output */
	if (!xout) {
		if (!(xout = realloc(bac->out.x, (oint+1) * sizeof(*xout)))) {
			return MPT_ERROR(BadOperation);
		}
		if (!bac->out.x) {
			double dx = 1.0 / oint;
			int i;
			for (i = 0; i <= oint; ++i) xout[i] = i * dx;
		}
		bac->out.x = xout;
	}
	/* use internal state output */
	if (!yout) {
		size_t l;
		int nder;
		l = bac->ivp.neqs * (oint+1) * sizeof(*yout);
		
		nder = deriv < 0 ? -deriv : deriv;
		
		l *= nder + 1;
		bac->out.nderiv = nder;
		
		if (!(yout = realloc(bac->out.y, l))) {
			return MPT_ERROR(BadOperation);
		}
		if (!bac->out.y && deriv < 0) {
			memset(yout, 0, l);
		}
		bac->out.y = yout;
	}
	/* adapt grid points to integrator intervals */
	if (bac->grid) {
		int len;
		if ((len = bac->grid(bac, oint+1, xout)) < 0) {
			return len;
		}
		if (len < 2) {
			return MPT_ERROR(BadValue);
		}
		return oint = len - 1;
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
		values_(&kcol, xout, &bac->nint, bac->x, &bac->ivp.neqs, &oint,
		        &deriv, yout, bac->y, bac->out.wrk.iov_base);
	}
	return oint + 1;
}


