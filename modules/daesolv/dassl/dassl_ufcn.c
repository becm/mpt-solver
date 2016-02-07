/*!
 * generic user functions dDASSL solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "dassl.h"

static void dassl_fcn(double *t, double *y, double *yp, double *f, int *ires, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(dassl) *data = (void *) rpar;
	int i, neqs;
	
	neqs = data->ivp.neqs;
	
	if ((*ires = ((const MPT_SOLVER_STRUCT(pdefcn) *) fcn)->fcn(fcn->dae.param, *t, y, f, &data->ivp, fcn->grid, fcn->rside)) < 0) {
		return;
	}
	if (!fcn->dae.mas) {
		neqs *= data->ivp.pint + 1;
		for (i = 0; i < neqs; i++) {
			f[i] -= yp[i];
		}
	}
	else {
		double *mas = data->dmas;
		int *idrow, *idcol, max, pint;
		
		max  = neqs * neqs;
		pint = data->ivp.pint;
		
		idrow = (int *) (mas + max);
		idcol = idrow + max;
		
		for (i = 0; i <= pint; ++i) {
			int j, nz;
			
			*idrow = max;
			*idcol = i;
			
			if ((nz = fcn->dae.mas(fcn->dae.param, *t, y+i*neqs, mas, idrow, idcol)) < 0) {
				*ires = nz;
				return;
			}
			/* f -= B*yp */
			for (j = 0; j < nz; j++) {
				f[i*neqs+idrow[j]] -= mas[j] * yp[i*neqs+idcol[j]];
			}
		}
	}
}

static void dassl_jac(double *t, double *y, double *ys, double *jac, double *cjp, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(daefcn) *dae = (void *) ipar;
	MPT_SOLVER_STRUCT(dassl) *data = (void *) rpar;
	double cj = *cjp;
	int ld, neqs, pint;
	
	(void) ys;
	
	neqs = data->ivp.neqs;
	pint = data->ivp.pint;
	
	if (!data->info[5]) {
		ld = neqs * (pint+1);
	}
	else {
		int ml, mu, *iwork = data->iwork.iov_base;
		
		ml = iwork[0];
		mu = iwork[1];
		/* irow = i - j + ml + mu + 1, ld = 2*ml + mu + 1 */
		jac += mu + ml;
		ld   = 2*ml + mu;
	}
	if (dae->jac(dae->param, *t, y, jac, ld) < 0) {
		abort();
	}
	/* Jac -= con*B */
	if (!dae->mas) {
		int i;
		neqs *= (pint+1);
		for (i = 0; i < neqs; i++, jac += ld) {
			jac[i] -= cj;
		}
	}
	else {
		double *mas = data->dmas;
		int nz, i, *idrow, *idcol;
		
		idrow = (int *) (mas + neqs * neqs);
		idcol = idrow + neqs * neqs;
		
		if ((nz = dae->mas(dae->param, *t, y, mas, idrow, idcol)) < 0) {
			abort();
		}
		/* Jac -= con*B */
		for (i = 0; i < nz; i++) {
			jac[(i*neqs+idcol[i])*ld+idrow[i]] -= cj * mas[i];
		}
	}
}

extern int mpt_dassl_ufcn(MPT_SOLVER_STRUCT(dassl) *da, const MPT_SOLVER_STRUCT(daefcn) *ufcn)
{
	if (!ufcn || !ufcn->fcn) {
		return MPT_ERROR(BadArgument);
	}
	
	if (da->ivp.pint) {
		da->jac = 0;
	} else {
		size_t nz = da->ivp.neqs * da->ivp.neqs;
		if (!da->dmas && !(da->dmas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
			return MPT_ERROR(BadOperation);
		}
		da->jac = ufcn->jac ? dassl_jac : 0;
	}
	da->fcn = dassl_fcn;
	
	da->ipar = (int *) ufcn;
	da->rpar = (double *) da;
	
	return 0;
}

