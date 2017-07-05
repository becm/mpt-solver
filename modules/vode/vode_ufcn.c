/*!
 * generic user functions dVODE solver instance
 */

#include <stdlib.h>

#include "vode.h"

static void vode_fcn(int *neq, double *t, double *y, double *f, double *rpar, int *ipar)
{
	const MPT_SOLVER_IVP_STRUCT(pdefcn) *pde = (void *) ipar;
	const MPT_SOLVER_STRUCT(vode) *vd = (void *) rpar;
	int ret;
	(void) neq;
	
	if (vd->ivp.pint) {
		ret = pde->fcn(pde->par, *t, y, f, &vd->ivp);
	} else {
		ret = pde->fcn(pde->par, *t, y, f, 0);
	}
	if (ret < 0) {
		abort();
	}
}

static void vode_jac(int *neq, double *t, double *y, int *ml, int *mu, double *jac, int *ljac, double *rpar, int *ipar)
{
	const MPT_SOLVER_IVP_STRUCT(odefcn) *ufcn = (void *) ipar;
	int ld;
	
	(void) rpar;
	
	if (*ml >= *neq) {
		ld = *ljac;
	} else {
		/* irow = i - j + MU + 1 */
		jac += *mu;
		ld   =  *ljac - 1;
	}
	if (ufcn->jac.fcn(ufcn->jac.par, *t, y, jac, ld) < 0) {
		abort();
	}
}

extern int mpt_vode_ufcn(MPT_SOLVER_STRUCT(vode) *vd, MPT_SOLVER_IVP_STRUCT(odefcn) *ufcn, int type, const void *ptr)
{
	int ret;
	if (!ptr) {
		switch (type) {
		  case MPT_SOLVER_ENUM(IvpRside):
		  case MPT_SOLVER_ENUM(IvpRside) | MPT_SOLVER_ENUM(PDE):
			if (ufcn) ufcn->rside.fcn = 0;
			break;
		  case MPT_SOLVER_ENUM(IvpJac):
			if (ufcn) ufcn->jac.fcn = 0;
			break;
		  case MPT_SOLVER_ENUM(ODE):
		  case MPT_SOLVER_ENUM(DAE):
		  case MPT_SOLVER_ENUM(PDE):
			if (ufcn) {
				ufcn->rside.fcn = 0;
				ufcn->jac.fcn = 0;
			}
			break;
		  default:
			return MPT_ERROR(BadType);
		}
	}
	else if (ufcn) {
		switch (type) {
		  case MPT_SOLVER_ENUM(IvpRside) | MPT_SOLVER_ENUM(PDE):
			if (!vd->ivp.pint) {
				return MPT_ERROR(BadType);
			}
			if (!((MPT_SOLVER_IVP_STRUCT(pdefcn) *) ptr)->fcn) {
				return MPT_ERROR(BadValue);
			}
			ufcn->rside = *((MPT_SOLVER_IVP_STRUCT(rside) *) ptr);
			break;
		  case MPT_SOLVER_ENUM(IvpRside):
			if (vd->ivp.pint) {
				return MPT_ERROR(BadType);
			}
			if (!((MPT_SOLVER_IVP_STRUCT(rside) *) ptr)->fcn) {
				return MPT_ERROR(BadValue);
			}
			ufcn->rside = *((MPT_SOLVER_IVP_STRUCT(rside) *) ptr);
			break;
		  case MPT_SOLVER_ENUM(IvpJac):
			ufcn->jac = *((MPT_SOLVER_IVP_STRUCT(jacobian) *) ptr);
			break;
		  case MPT_SOLVER_ENUM(ODE):
			*ufcn = *((MPT_SOLVER_IVP_STRUCT(odefcn) *) ptr);
			break;
		  case MPT_SOLVER_ENUM(DAE):
			if (((MPT_SOLVER_IVP_STRUCT(daefcn) *) ptr)->mas.fcn) {
				return MPT_ERROR(BadValue);
			}
			*ufcn = *((MPT_SOLVER_IVP_STRUCT(odefcn) *) ptr);
			break;
		  default:
			return MPT_ERROR(BadType);
		}
	}
	vd->fcn = 0;
	vd->jac = 0;
	if (!ufcn) {
		vd->ipar = 0;
		vd->rpar = 0;
		return 0;
	}
	ret = 0;
	if (ufcn->rside.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpRside);
		vd->fcn = vode_fcn;
	}
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(IvpJac);
		vd->jac = vode_jac;
	}
	if (vd->ivp.pint) {
		ret |= MPT_SOLVER_ENUM(PDE);
	}
	vd->ipar = (int *) ufcn;
	vd->rpar = (double *) vd;
	
	return ret;
}

