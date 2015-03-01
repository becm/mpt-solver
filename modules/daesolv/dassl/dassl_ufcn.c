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
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	MPT_SOLVER_STRUCT(dassl) *data;
	int i, neqs, pint;
	
	fcn  = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	
	if ((*ires = fcn->fcn(fcn->param, t, y, f)) < 0) {
		return;
	}
	data = (MPT_SOLVER_STRUCT(dassl) *) rpar;
	neqs = data->ivp.neqs;
	pint = data->ivp.pint;
	
	if (!fcn->mas) {
		neqs *= pint + 1;
		for (i = 0; i < neqs; i++) {
			f[i] -= yp[i];
		}
	}
	else {
		double *mas = data->dmas;
		int *idrow, *idcol;
		
		idrow = (int *) (mas + neqs * neqs);
		idcol = idrow + neqs * neqs;
		
		for (i = 0; i <= pint; i++) {
			int nz, j;
			
			*idrow = i;
			
			if ((nz = fcn->mas(fcn->param, t, y, mas, idrow, idcol)) < 0) {
				*ires = nz;
				return;
			}
			/* f -= B*yp */
			for (j = 0; j < nz; j++) {
				f[idrow[j]] -= mas[j] * yp[idcol[j]];
			}
			y += neqs;
			f += neqs;
		}
	}
}

static void dassl_jac(double *t, double *y, double *ys, double *jac, double *cj, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	MPT_SOLVER_STRUCT(dassl) *data;
	int ld, neqs, pint;
	
	(void) ys;
	fcn  = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	data = (MPT_SOLVER_STRUCT(dassl) *) rpar;
	
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
	if (fcn->jac(fcn->param, t, y, jac, ld) < 0) {
		abort();
	}
	/* Jac -= con*B */
	if (!fcn->mas) {
		int i;
		neqs *= (pint+1);
		for (i = 0; i < neqs; i++, jac += ld) {
			jac[i] -= *cj;
		}
	}
	else {
		double *mas = data->dmas;
		int i, *idrow, *idcol;
		
		idrow = (int *) (mas + neqs * neqs);
		idcol = idrow + neqs * neqs;
		
		for (i = 0; i <= pint; i++) {
			int nz, j;
			
			*idrow = i;
			
			if ((nz = fcn->mas(fcn->param, t, y, mas, idrow, idcol)) < 0)
				abort();
			
			/* Jac -= con*B */
			for (j = 0; j < nz; j++) {
				jac[(i*neqs+idcol[i])*ld+idrow[i]] -= (*cj) * mas[i];
			}
			y += neqs;
		}
	}
}

extern int mpt_dassl_ufcn(MPT_SOLVER_STRUCT(dassl) *data, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	if (!ufcn->fcn) {
		errno = EFAULT;
		return -1;
	}
	/* need temporal mass matrix */
	if (ufcn->mas) {
		double *mas;
		int mlen = sizeof(double) + 2 * sizeof(int);
		
		mlen *= data->ivp.neqs * data->ivp.neqs;
		
		if (!mlen || !(mas = realloc(data->dmas, mlen))) {
			return -1;
		}
		data->dmas = mas;
	}
	data->ipar = (int *) ufcn;
	data->rpar = (double *) data;
	
	data->jac = ufcn->jac ? dassl_jac : 0;
	data->fcn = dassl_fcn;
	
	return 0;
}

