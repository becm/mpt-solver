/*!
 * create output values for BACOL descriptor
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern void uinit_(const double *, double *, const int *);

extern double *mpt_bacol_values(MPT_SOLVER_STRUCT(bacolout) *out, const MPT_SOLVER_STRUCT(bacol) *bac)
{
	double *x, *y;
	int wlen, kcol, nint, npts, neqs, deriv;
	
	if (!out) {
		return 0;
	}
	if (out->nint) {
		x = out->xy.iov_base;
		return x + out->nint + 1;
	}
	if (!bac || bac->mflag.noinit < 0 || (nint = bac->ivp.pint) < 0) {
		return 0;
	}
	if (!nint) {
		if (!out->update) {
			return 0;
		}
		nint = bac->nint * bac->kcol;
	}
	kcol = bac->kcol;
	neqs = bac->ivp.neqs;
	deriv = out->deriv;
	
	/* check output work data availability */
	wlen = (kcol * MPT_BACOL_NCONTI) * (1 + deriv);
	wlen += kcol * (bac->nint + 1) + 2 * MPT_BACOL_NCONTI;
	
	if (!mpt_vecpar_alloc(&out->wrk, wlen, sizeof(double))) {
		return 0;
	}
	npts = nint + 1;
	if (!(x = mpt_vecpar_alloc(&out->xy, npts + npts * (deriv + 1) * neqs, sizeof(double)))) {
		return 0;
	}
	if (out->update && (nint = out->update(bac->nint, bac->xy, nint, x)) < 0) {
		return 0;
	}
	npts = nint + 1;
	y = x + npts;
	
	if (bac->mflag.noinit) {
		/* generate y-values for grid */
		values_(&kcol, x, &bac->nint, bac->xy, &neqs, &npts,
		        &deriv, y, bac->xy + bac->nintmx + 1, out->wrk.iov_base);
	} else {
		int pos;
		
		for (pos = 0; pos < npts; ++pos) {
			uinit_(x+pos, y+pos*neqs, &neqs);
		}
	}
	out->nint = nint;
	out->neqs = neqs;
	return y;
}


