/*!
 * generic user functions dVODE solver instance
 */

#include <stdlib.h>
#include <errno.h>

#include "vode.h"

static void vode_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	
	(void) neq;
	(void) rpar;
	
	if (ufcn->fcn(ufcn->param, t, y, f) < 0) {
		abort();
	}
}

static void vode_jac(int *neq, double *t, double *y, int *ml, int *mu, double *jac, int *ljac, double *rpar, int *ipar)
{
	MPT_SOLVER_STRUCT(ivpfcn) *ufcn;
	int ld;
	
	(void) rpar;
	ufcn = (MPT_SOLVER_STRUCT(ivpfcn) *) ipar;
	
	if (*ml >= *neq)
		ld = *ljac;
	else {
		/* irow = i - j + MU + 1 */
		jac += *mu;
		ld   =  *ljac -1;
	}
	if (ufcn->jac(ufcn->param, t, y, jac, ld) < 0)
		abort();
}

extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *data, MPT_SOLVER_STRUCT(ivpfcn) *ufcn)
{
	if (!ufcn->fcn) {
		errno = EFAULT; return -1;
	}
	if (ufcn->mas) {
		errno = EINVAL; return -3;
	}
	data->jac = ufcn->jac ? vode_jac : 0;
	data->fcn = vode_fcn;
	data->ipar = (int *) ufcn;
	
	return 0;
}

