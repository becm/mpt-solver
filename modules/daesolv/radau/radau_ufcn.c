/*!
 * generic user functions RADAU solver instance
 */

#include <stdlib.h>

#include "radau.h"

static void radau_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) rpar;
	MPT_SOLVER_IVP_STRUCT(functions) *fcn = (void *) ipar;
	MPT_SOLVER_IVP_STRUCT(parameters) ivp = rd->ivp;
	
	(void) neq;
	
	if (((MPT_SOLVER_STRUCT(pdefcn) *) fcn)->fcn(fcn->dae.param, *t, y, f, &ivp, fcn->grid, fcn->rside) < 0) {
		abort();
	}
}

static void radau_jac(int *neq, double *t, double *y, double *jac, int *ljac, double *rpar, int *ipar)
{
	MPT_SOLVER_IVP_STRUCT(functions) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(radau) *rd = (void *) rpar;
	int ld;
	
	(void) neq;
	
	if (rd->mljac >= *neq) {
		ld = *ljac;
	}
	else {
		/* irow = i - j + mujac + 1 */
		jac += rd->mujac;
		ld   = *ljac - 1;
	}
	if (fcn->dae.jac(fcn->dae.param, *t, y, jac, ld) < 0) {
		abort();
	}
}

static void radau_mas(int *neq, double *b, int *lmasp, double *rpar, int *ipar)
{
	MPT_SOLVER_IVP_STRUCT(functions) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(radau) *rd = (void *) rpar;
	double *mas;
	int *idrow, *idcol;
	int i, pint, max, lmas, dmas;
	
	lmas = *lmasp;
	dmas = lmas * (*neq);
	
	max  = rd->ivp.neqs * rd->ivp.neqs;
	pint = rd->ivp.pint;
	
	mas = rd->dmas;
	idrow = (int *) (mas + max);
	idcol = idrow + max;
	
	for (i = 0; i <= pint; ++i) {
		int j, nz;
		
		idrow[0] = max;
		idcol[0] = i;
		
		if ((nz = fcn->dae.mas(fcn->dae.param, rd->t, 0, mas, idrow, idcol)) < 0) {
			abort();
		}
		for (j = 0; j < nz; j++) {
			int pos = idrow[j] + lmas * idcol[j];
			if (pos < dmas) {
				b[pos] = mas[j];
			}
		}
	}
}

extern int mpt_radau_ufcn(MPT_SOLVER_STRUCT(radau) *rd, const MPT_SOLVER_IVP_STRUCT(functions) *ufcn)
{
	
	if (!ufcn || !ufcn->dae.fcn) {
		return MPT_ERROR(BadArgument);
	}
	if (ufcn->dae.mas) {
		double *mas;
		size_t nz = rd->ivp.neqs * rd->ivp.neqs;
		
		if (!nz) {
			return MPT_ERROR(BadArgument);
		}
		if (!(mas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
			return MPT_ERROR(BadOperation);
		}
		if (rd->dmas) {
			free(rd->dmas);
		}
		rd->dmas = mas;
	}
	rd->fcn = radau_fcn;
	rd->jac = ufcn->dae.jac ? radau_jac : 0;
	rd->mas = ufcn->dae.mas ? radau_mas : 0;
	
	rd->ipar = (int *) ufcn;
	rd->rpar = (double *) rd;
	
	return 0;
}

