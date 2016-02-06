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
	MPT_SOLVER_STRUCT(daefcn) *dae = (void *) ipar;
	MPT_SOLVER_STRUCT(dassl) *data = (void *) rpar;
	int i, neqs;
	
	neqs = data->ivp.neqs;
	
	if (data->ivp.pint) {
		MPT_SOLVER_STRUCT(pdefcn) *pde = (void *) rpar;
		*ires = pde->fcn(pde->param, *t, y, f, &data->ivp, pde->grid, pde->rside);
		return;
	}
	if ((*ires = dae->fcn(dae->param, *t, y, f)) < 0) {
		return;
	}
	if (!dae->mas) {
		for (i = 0; i < neqs; i++) {
			f[i] -= yp[i];
		}
	}
	else {
		double *mas = data->dmas;
		int *idrow, *idcol, nz;
		
		nz = neqs * neqs;
		
		idrow = (int *) (mas + nz);
		idcol = idrow + nz;
		
		*idrow = nz;
		*idcol = neqs;
		
		if ((nz = dae->mas(dae->param, *t, y, mas, idrow, idcol)) < 0) {
			*ires = nz;
			return;
		}
		/* f -= B*yp */
		for (i = 0; i < nz; i++) {
			f[idrow[i]] -= mas[i] * yp[idcol[i]];
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

