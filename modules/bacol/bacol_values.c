/*!
 * create output values for BACOL descriptor
 */

#include <stdlib.h>
#include <string.h>

#include "bacol.h"

extern void uinit_(const double *, double *, const int *);

extern const double *mpt_bacol_values(MPT_SOLVER_STRUCT(bacol_out) *out, const MPT_SOLVER_STRUCT(bacol) *bac)
{
	double *val, *wrk;
	int wlen, kcol, nint, npts, neqs, deriv;
	
	if (!bac || bac->mflag.noinit < 0) {
		return 0;
	}
	if (!(nint = bac->ivp.pint)) {
		return 0;
	}
	kcol = bac->kcol;
	neqs = bac->ivp.neqs;
	
	deriv = out->deriv;
	
	/* check output work data availability */
	wlen = (kcol * MPT_BACOL_NCONTI) * (1 + deriv);
	wlen += kcol * (bac->nint + 1) + 2 * MPT_BACOL_NCONTI;
	
	if (!(wrk = mpt_solver_module_valloc(&out->_wrk, wlen, sizeof(double)))) {
		return 0;
	}
	npts = nint + 1;
	wlen = npts + npts * (deriv + 1) * neqs;
	
	if (!(val = mpt_solver_module_valloc(&out->_val, wlen, sizeof(double)))) {
		return 0;
	}
	if (out->update && bac->xy) {
		int intv;
		if ((intv = out->update(bac->nint, bac->xy, nint, val)) < 0
		 || intv > nint) {
			return 0;
		}
		nint = intv;
		npts = nint + 1;
	}
	else if (bac->ivp.grid) {
		memcpy(val, bac->ivp.grid, npts * sizeof(*val));
	}
	else {
		nint = mpt_bacol_grid_init(0, 0, nint, val);
	}
	if (bac->mflag.noinit) {
		/* generate y-values for grid */
		values_(&kcol, val, &bac->nint, bac->xy, &neqs, &npts,
		        &deriv, val + npts, bac->xy + bac->nintmx + 1, wrk);
	} else {
		int pos;
		double *y = val + npts;
		for (pos = 0; pos < npts; ++pos) {
			uinit_(val + pos, y + pos * neqs, &neqs);
		}
	}
	out->nint = nint;
	out->neqs = neqs;
	return val;
}


