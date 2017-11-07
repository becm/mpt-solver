/*!
 * generic user functions RADAU solver instance
 */

#include <stdlib.h>

#include "radau.h"

#include "module_functions.h"

static void radau_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(radau) *rd = (void *) rpar;
	MPT_IVP_STRUCT(pdefcn) *pde = (void *) ipar;
	
	(void) neq;
	
	if (pde->fcn(pde->par, *t, y, f, &rd->ivp) < 0) {
		abort();
	}
}

static void radau_jac(int *neq, double *t, double *y, double *jac, int *ljac, double *rpar, int *ipar)
{
	MPT_IVP_STRUCT(daefcn) *fcn = (void *) ipar;
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
	if (fcn->jac.fcn(fcn->jac.par, *t, y, jac, ld) < 0) {
		abort();
	}
}

static void radau_mas(int *neq, double *b, int *lmasp, double *rpar, int *ipar)
{
	MPT_IVP_STRUCT(daefcn) *fcn = (void *) ipar;
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
		
		if ((nz = fcn->mas.fcn(fcn->mas.par, rd->t, 0, mas, idrow, idcol)) < 0) {
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

extern int mpt_radau_ufcn(MPT_SOLVER_STRUCT(radau) *rd, MPT_IVP_STRUCT(daefcn) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if ((ret = mpt_solver_module_ufcn_dae(rd->ivp.pint, ufcn, type, ptr)) < 0) {
		return ret;
	}
	rd->fcn = 0;
	rd->jac = 0;
	rd->mas = 0;
	if (!ufcn) {
		rd->ipar = 0;
		rd->rpar = 0;
		return 0;
	}
	ret = 0;
	if (ufcn->mas.fcn) {
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
		rd->mas = radau_mas;
	}
	if (ufcn->jac.fcn) {
		rd->jac = radau_jac;
	}
	if (ufcn->jac.fcn) {
		rd->fcn = radau_fcn;
	}
	rd->ipar = (int *) ufcn;
	rd->rpar = (double *) rd;
	
	return ret;
}

