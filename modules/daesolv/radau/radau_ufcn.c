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
	MPT_SOLVER_TYPE(ivpfcn) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(radau) *rd = (void *) rpar;
	
	(void) neq;
	(void) rpar;
	
	if (rd->ivp.pint) {
		MPT_SOLVER_STRUCT(pdefcn) *pde = &fcn->pde;
		MPT_SOLVER_STRUCT(ivppar) ivp = rd->ivp;
		if (pde->fcn(pde->param, *t, y, f, &ivp, pde->grid, pde->rside) < 0) {
			abort();
		}
		return;
	}
	if (fcn->dae.fcn(fcn->dae.param, *t, y, f) < 0) {
		abort();
	}
}

static void radau_jac(int *neq, double *t, double *y, double *jac, int *ljac, double *rpar, int *ipar)
{
	MPT_SOLVER_TYPE(ivpfcn) *fcn = (void *) ipar;
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
	MPT_SOLVER_TYPE(ivpfcn) *fcn = (void *) ipar;
	MPT_SOLVER_STRUCT(radau) *rd = (void *) rpar;
	double *mas;
	int *idrow, *idcol;
	int i, nz, lmas, dmas;
	
	lmas = *lmasp;
	dmas = lmas * (*neq);
	nz = rd->ivp.neqs * rd->ivp.neqs;
	
	mas = rd->dmas;
	idrow = (int *) (mas + nz);
	idcol = idrow + nz;
	
	*idrow = nz;
	*idcol = rd->ivp.neqs;
	
	if ((nz = fcn->dae.mas(fcn->dae.param, rd->t, 0, mas, idrow, idcol)) < 0) {
		abort();
	}
	for (i = 0; i < nz; i++) {
		int pos = idrow[i] + lmas * idcol[i];
		if (pos < dmas) {
			b[pos] = mas[i];
		}
	}
}

extern int mpt_radau_ufcn(MPT_SOLVER_STRUCT(radau) *rd, const MPT_SOLVER_STRUCT(daefcn) *ufcn)
{
	if (!ufcn || !ufcn->fcn) {
		return MPT_ERROR(BadArgument);
	}
	if (rd->ivp.pint) {
		rd->jac = 0;
		rd->mas = 0;
	} else {
		size_t nz = rd->ivp.neqs * rd->ivp.neqs;
		if (!rd->dmas && !(rd->dmas = malloc(nz * (2 * sizeof(int) + sizeof(double))))) {
			return MPT_ERROR(BadOperation);
		}
		rd->jac = ufcn->jac ? radau_jac : 0;
		rd->mas = ufcn->mas ? radau_mas : 0;
	}
	rd->fcn = radau_fcn;
	
	rd->ipar = (int *) ufcn;
	rd->rpar = (double *) rd;
	
	return 0;
}

