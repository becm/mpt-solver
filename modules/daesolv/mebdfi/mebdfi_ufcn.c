/*!
 * generic user functions MEBDFI solver instance
 */

#include <stdlib.h>

#include "mebdfi.h"

#include "solver_daefcn.h"

static void mebdfi_fcn(int *neq, double *t, double *y, double *f, double *yp, int *ipar, double *rpar, int *flg)
{
	MPT_SOLVER_IVP_STRUCT(daefcn) *fcn = (void *) ipar;
	MPT_SOLVER_IVP_STRUCT(pdefcn) *pde = (void *) ipar;
	MPT_SOLVER_STRUCT(mebdfi) *me = (void *) rpar;
	
	if ((*flg = pde->fcn(pde->par, *t, y, f, &me->ivp)) < 0) {
		return;
	}
	if (!fcn->mas.fcn) {
		int i, neqs = *neq;
		for (i = 0; i < neqs; i++) {
			f[i] -= yp[i];
		}
	}
	else {
		double *mas;
		int *idrow, *idcol, nz, i;
		
		nz = me->ivp.neqs * me->ivp.neqs;
		
		mas = me->dmas;
		idrow = (int *) (mas + nz);
		idcol = idrow + nz;
		
		*idrow = nz;
		*idcol = me->ivp.neqs;
		
		if ((nz = fcn->mas.fcn(fcn->mas.par, *t, y, mas, idrow, idcol)) < 0) {
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
	MPT_SOLVER_IVP_STRUCT(daefcn) *fcn = (void *) ipar;
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
	*info = fcn->jac.fcn(fcn->jac.par, *t, y, jac, ljac);
	
	/* Jac -= con*B */
	if (!fcn->mas.fcn) {
		++ljac;
		for (i = 0; i < neqs; i++) {
			jac[i * ljac] -= *con;
		}
	}
	else {
		MPT_SOLVER_STRUCT(mebdfi) *me = (MPT_SOLVER_STRUCT(mebdfi) *) rpar;
		double *mas, c = *con;
		int *idrow, *idcol, nz, i, pint;
		
		neqs = me->ivp.neqs;
		pint = me->ivp.pint;
		nz   = neqs * neqs;
		
		mas   = me->dmas;
		idrow = (int *) (mas + nz);
		idcol = idrow + nz;
		
		for (i = 0; i <= pint; ++i) {
			int j;
			
			idrow[0] = nz;
			idcol[0] = i;
			
			if ((nz = fcn->mas.fcn(fcn->mas.par, *t, y, mas, idrow, idcol)) < 0) {
				*info = nz;
				return;
			}
			for (j = 0; j < nz ; j++) {
				jac[idcol[j]*ljac+idrow[j]] -= c * mas[j];
			}
			y += neqs;
		}
	}
}

extern int mpt_mebdfi_ufcn(MPT_SOLVER_STRUCT(mebdfi) *me, MPT_SOLVER_IVP_STRUCT(daefcn) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if ((ret = _mpt_solver_daefcn_set(me->ivp.pint, ufcn, type, ptr)) < 0) {
		return ret;
	}
	me->fcn = 0;
	me->jac = 0;
	
	if (!ufcn) {
		me->ipar = 0;
		me->rpar = 0;
		return 0;
	}
	if (ufcn->mas.fcn) {
		double *mas;
		size_t nz = me->ivp.neqs * me->ivp.neqs;
		
		if (!nz) {
			return MPT_ERROR(BadArgument);
		}
		if (!(mas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
			return MPT_ERROR(BadOperation);
		}
		if (me->dmas) {
			free(me->dmas);
		}
		me->dmas = mas;
	}
	if (ufcn->jac.fcn) {
		me->jac = mebdfi_jac;
	}
	if (ufcn->rside.fcn) {
		me->fcn = mebdfi_fcn;
	}
	me->ipar = (int *) ufcn;
	me->rpar = (double *) me;
	
	return ret;
}

