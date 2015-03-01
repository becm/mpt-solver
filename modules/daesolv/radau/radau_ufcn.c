/*!
 * generic user functions RADAU solver instance
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "radau.h"

static void radau_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	
	(void) neq;
	(void) rpar;
	fcn = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	
	if (fcn->fcn(fcn->param, t, y, f) < 0) {
		abort();
	}
}

static void radau_jac(int *neq, double *t, double *y, double *jac, int *ljac, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	MPT_SOLVER_STRUCT(radau)  *data;
	int ld;
	
	(void) neq;
	fcn  = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	data = (MPT_SOLVER_STRUCT(radau) *) rpar;
	
	if (data->mljac >= *neq) {
		ld = *ljac;
	}
	else {
		/* irow = i - j + mujac + 1 */
		jac += data->mujac;
		ld   = *ljac - 1;
	}
	if (fcn->jac(fcn->param, t, y, jac, ld) < 0)
		abort();
}

static void radau_mas(int *neq, double *b, int *lmas, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *fcn;
	MPT_SOLVER_STRUCT(radau) *data;
	double *mas;
	int *idrow, *idcol;
	int i, nl, dmas = (*lmas) * (*neq);
	
	fcn  = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	data = (MPT_SOLVER_STRUCT(radau) *) rpar;
	mas  = data->dmas;
	
	nl = data->ivp.neqs * data->ivp.neqs;
	
	idrow = (int *) (mas + nl);
	idcol = idrow + nl;
	
/* 	(void) memset(b, 0, sizeof(*b) * dmas);*/
	
	nl = data->ivp.pint;
	
	for (i = 0; i <= nl; i++) {
		int j, nb;
		
		*idrow = i;
		
		if ((nb = fcn->mas(fcn->param, &data->ivp.last, 0, mas, idrow, idcol)) < 0) {
			abort();
		}
		for (j = 0; j < nb; j++) {
			int pos = idrow[i] + (*lmas) * idcol[i];
			if (pos < dmas) b[pos] = mas[i];
		}
	}
}

extern int mpt_radau_ufcn(MPT_SOLVER_STRUCT(radau) *data, const MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	if (!ufcn->fcn) {
		errno = EFAULT;
		return -1;
	}
	/* need temporal mass matrix */
	if (!ufcn->mas) {
		data->mas = 0;
	}
	else {
		double	*mas;
		size_t	mlen = sizeof(double) + 2 * sizeof(int);
		mlen *= data->ivp.neqs * data->ivp.neqs;
		
		if (!(mas = realloc(data->dmas, mlen))) {
			return -1;
		}
		data->dmas = mas;
	}
	
	data->jac = ufcn->jac ? radau_jac : 0;
	data->mas = ufcn->mas ? radau_mas : 0;
	data->fcn = radau_fcn;
	
	data->ipar = (int *) ufcn;
	data->rpar = (data->jac || data->mas) ? (double *) data : 0;
	
	return 0;
}

