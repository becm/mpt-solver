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
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	
	fcn = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	
	if ( (*flg = fcn->fcn(fcn->param, t, y, f)) < 0 )
		return;
	
	if ( !fcn->mas ) {
		int	i, neqs = *neq;
		for (i = 0; i < neqs; i++)
			f[i] -= yp[i];
	}
	else {
		MPT_SOLVER_STRUCT(mebdfi) *meb = (MPT_SOLVER_STRUCT(mebdfi) *) rpar;
		double *mas = meb->dmas;
		int *idrow, *idcol, neqs, nint, i;
		
		neqs = meb->ivp.neqs;
		nint = meb->ivp.pint;
		
		idrow = (int *) (mas + neqs * neqs);
		idcol = idrow + neqs * neqs;
		
		for (i = 0; i <= nint; i++) {
			int	nz, j;
			
			*idrow = i;
			
			if ((nz = fcn->mas(fcn->param, t, y, mas, idrow, idcol)) < 0) {
				*flg = nz; return;
			}
			/* f -= B*yp */
			for (j = 0; j < nz ; j++)
				f[idrow[j]] -= mas[j] * yp[idcol[j]];
			
			y += neqs;
		}
	}
}

static void mebdfi_jac(double *t, double *y, double *jac, int *neq, double *yp, int *mbnd, double *con, int *ipar, double *rpar, int *info)
{
	MPT_SOLVER_STRUCT(ivpfcn) *fcn = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
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
	*info = fcn->jac(fcn->param, t, y, jac, ljac);
	
	/* Jac -= con*B */
	if ( !fcn->mas ) {
		for (i = 0; i < neqs; i++, jac += ljac) {
			jac[i] -= *con;
		}
	}
	else {
		MPT_SOLVER_STRUCT(mebdfi) *meb = (MPT_SOLVER_STRUCT(mebdfi) *) rpar;
		double *mas = meb->dmas;
		int *idrow, *idcol, nint, i;
		
		neqs = meb->ivp.neqs;
		nint = meb->ivp.pint;
		
		idrow = (int *) (mas + neqs * neqs);
		idcol = idrow + neqs * neqs;
		
		for (i = 0; i <= nint; i++) {
			int	nz, j;
			
			*idrow = i;
			
			if ((nz = fcn->mas(fcn->param, t, y, mas, idrow, idcol)) < 0) {
				*info = nz; return;
			}
			for (j = 0; j < nz ; j++)
				jac[idcol[j]*ljac+idrow[j]] -= (*con) * mas[j];
			
			y += neqs;
		}
	}
}

extern int mpt_mebdfi_ufcn(MPT_SOLVER_STRUCT(mebdfi) *data, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	if ( !ufcn->fcn ) {
		errno = EFAULT; return -1;
	}
	/* need temporal mass matrix */
	if (ufcn->mas) {
		double	*mas;
		size_t	mlen = sizeof(double) + 2 * sizeof(int);
		
		mlen *= data->ivp.neqs * data->ivp.neqs;
		
		if (!mlen || !(mas = realloc(data->dmas, mlen)) )
			return -1;
		
		data->dmas = mas;
	}
	data->ipar = (int *) ufcn;
	data->rpar = (double *) data;
	
	data->jac = ufcn->jac ? mebdfi_jac : 0;
	data->fcn = mebdfi_fcn;
	
	return 0;
}

