/*!
 * generic user functions MEBDFI solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mebdfi.h"

static void mebdfi_fcn(int *neq, double *t, double *y, double *f, double *yp, int *ipar, double *rpar, int *flg)
{
	MPT_SOLVER_TYPE(ivpfcn) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) rpar;
	
	if (me->ivp.pint) {
		*flg = fcn->pde.fcn(fcn->pde.param, *t, y, f, &me->ivp, fcn->pde.grid, fcn->pde.rside);
		return;
	}
	if ((*flg = fcn->dae.fcn(fcn->dae.param, *t, y, f)) < 0) {
		return;
	}
	if (!fcn->dae.mas) {
		int i, neqs = *neq;
		for (i = 0; i < neqs; i++) {
			f[i] -= yp[i];
		}
	}
	else {
		double *mas;
		int *idrow, *idcol, nz, i;
		
		nz = me->ivp.neqs * me->ivp.neqs;
		
		if (!(mas = me->dmas)) {
			if (!(mas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
				*flg = MPT_ERROR(BadOperation);
				return;
			}
			me->dmas = mas;
		}
		idrow = (int *) (mas + nz);
		idcol = idrow + nz;
		
		*idrow = nz;
		*idcol = me->ivp.neqs;
		
		if ((nz = fcn->dae.mas(fcn->dae.param, *t, y, mas, idrow, idcol)) < 0) {
			*flg = nz;
			return;
		}
		/* f -= B*yp */
		for (i = 0; i < nz ; i++) {
			f[idrow[i]] -= mas[i] * yp[idcol[i]];
		}
	}
}

static void mebdfi_jac(double *t, double *y, double *jac, int *neq, double *yp, int *mbnd, double *con, int *ipar, double *rpar, int *info)
{
	MPT_SOLVER_TYPE(ivpfcn) *fcn = (void *) ipar;
	int i, neqs = *neq, ljac;
	
	(void) yp;
	
	if (mbnd[0] >= neqs) {
		ljac = neqs;
	}
	else {
		/* row = i-j+mu+1, ld = mbnd(4) */
		jac += mbnd[1];
		ljac = mbnd[3];
	}
	*info = fcn->dae.jac(fcn->dae.param, *t, y, jac, ljac);
	
	/* Jac -= con*B */
	if (!fcn->dae.mas) {
		++ljac;
		for (i = 0; i < neqs; i++) {
			jac[i * ljac] -= *con;
		}
	}
	else {
		MPT_SOLVER_STRUCT(mebdfi) *me = (MPT_SOLVER_STRUCT(mebdfi) *) rpar;
		double *mas, c = *con;
		int *idrow, *idcol, nz, i;
		
		nz = me->ivp.neqs * me->ivp.neqs;
		
		if (!(mas = me->dmas)) {
			if (!(mas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
				*info = MPT_ERROR(BadOperation);
			}
			return;
		}
		idrow = (int *) (mas + nz);
		idcol = idrow + nz;
		
		*idrow = nz;
		*idcol = me->ivp.neqs;
		
		if ((nz = fcn->dae.mas(fcn->dae.param, *t, y, mas, idrow, idcol)) < 0) {
			*info = nz;
			return;
		}
		for (i = 0; i < nz ; i++) {
			jac[idcol[i]*ljac+idrow[i]] -= c * mas[i];
		}
	}
}

extern int mpt_mebdfi_ufcn(MPT_SOLVER_STRUCT(mebdfi) *me, const MPT_SOLVER_STRUCT(daefcn) *ufcn)
{
	if (!ufcn || !ufcn->fcn) {
		return MPT_ERROR(BadArgument);
	}
	if (me->ivp.pint) {
		me->jac = 0;
	} else {
		size_t nz = me->ivp.neqs * me->ivp.neqs;
		if (!me->dmas && !(me->dmas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
			return MPT_ERROR(BadOperation);
		}
		me->jac = ufcn->jac ? mebdfi_jac : 0;
	}
	me->fcn = mebdfi_fcn;
	
	me->ipar = (int *) ufcn;
	me->rpar = (double *) me;
	
	return 0;
}

