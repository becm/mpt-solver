/*!
 * generic user functions dDASSL solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dassl.h"

#include "solver_daefcn.h"

static void dassl_fcn(double *t, double *y, double *yp, double *f, int *ires, double *rpar, int *ipar)
{
	MPT_IVP_STRUCT(daefcn) *fcn = (void *) ipar;
	MPT_IVP_STRUCT(pdefcn) *pde = (void *) ipar;
	MPT_SOLVER_STRUCT(dassl) *data = (void *) rpar;
	int i, neqs;
	
	neqs = data->ivp.neqs;
	
	if ((*ires = (pde->fcn(pde->par, *t, y, f, &data->ivp)) < 0)) {
		return;
	}
	if (!fcn->mas.fcn) {
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
			
			if ((nz = fcn->mas.fcn(fcn->mas.par, *t, y + i * neqs, mas, idrow, idcol)) < 0) {
				*ires = nz;
				return;
			}
			/* f -= B*yp */
			for (j = 0; j < nz; j++) {
				f[i * neqs + idrow[j]] -= mas[j] * yp[i * neqs + idcol[j]];
			}
		}
	}
}

static void dassl_jac(double *t, double *y, double *ys, double *jac, double *cjp, double *rpar, int *ipar)
{
	MPT_IVP_STRUCT(daefcn) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(dassl) *data = (void *) rpar;
	double cj = *cjp;
	long ld, neqs, pint;
	
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
		ld   = 2 * ml + mu;
	}
	if (fcn->jac.fcn(fcn->jac.par, *t, y, jac, ld) < 0) {
		abort();
	}
	/* Jac -= con*B */
	if (!fcn->mas.fcn) {
		long i;
		neqs *= (pint + 1);
		for (i = 0; i < neqs; i++, jac += ld) {
			jac[i] -= cj;
		}
	}
	else {
		double *mas = data->dmas;
		int nz, i, *idrow, *idcol;
		
		idrow = (int *) (mas + neqs * neqs);
		idcol = idrow + neqs * neqs;
		
		if ((nz = fcn->mas.fcn(fcn->mas.par, *t, y, mas, idrow, idcol)) < 0) {
			abort();
		}
		/* Jac -= con*B */
		for (i = 0; i < nz; i++) {
			jac[idcol[i] * ld + idrow[i]] -= cj * mas[i];
		}
	}
}

extern int mpt_dassl_ufcn(MPT_SOLVER_STRUCT(dassl) *da, MPT_IVP_STRUCT(daefcn) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if ((ret = _mpt_solver_daefcn_set(da->ivp.pint, ufcn, type, ptr)) < 0) {
		return ret;
	}
	da->fcn = 0;
	da->jac = 0;
	if (!ufcn) {
		da->ipar = 0;
		da->rpar = 0;
		return 0;
	}
	if (ufcn->mas.fcn) {
		double *mas;
		size_t nz = da->ivp.neqs * da->ivp.neqs;
		
		if (!nz) {
			return MPT_ERROR(BadArgument);
		}
		if (!(mas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
			return MPT_ERROR(BadOperation);
		}
		if (da->dmas) {
			free(da->dmas);
		}
		da->dmas = mas;
	}
	if (ufcn->jac.fcn) {
		da->jac = dassl_jac;
	}
	if (ufcn->rside.fcn) {
		da->fcn = dassl_fcn;
	}
	da->ipar = (int *) ufcn;
	da->rpar = (double *) da;
	
	return 0;
}

