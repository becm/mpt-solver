/*!
 * generic user function wrapper for PORT N2 instance
 */

#include "portn2.h"

static void portn2_fcn(const int *n, const int *p, const double *x, int *nf, double *r, int *ui, double *ur, void (*uf)())
{
	MPT_SOLVER_NLS_STRUCT(functions) *ufcn = (void *) ui;
	int ld[2];
	
	(void) ur;
	(void) uf;
	
	ld[0] = *n;
	ld[1] = *p;
	
	if (ufcn->res.fcn(ufcn->res.par, x, r, ld) < 0) {
		*nf = 0;
	}
}

static void portn2_jac(const int *n, const int *p, const double *x, int *nf, double *jac, int *ui, double *ur, void (*uf)())
{
	MPT_SOLVER_NLS_STRUCT(functions) *ufcn = (void *) ui;
	int ld[3];
	
	(void) ur;
	(void) uf;
	
	ld[0] = *n;
	ld[1] = *n;
	ld[2] = *p;
	
	if (ufcn->jac.fcn(ufcn->jac.par, x, jac, ld, 0) < 0) {
		*nf = 0;
	}
}

extern int mpt_portn2_ufcn(MPT_SOLVER_STRUCT(portn2) *n2, MPT_SOLVER_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	int ret;
	if (!ptr) {
		switch (type) {
		  case 0:
			return MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsJac);
		  case MPT_SOLVER_ENUM(NlsVector):
			if (ufcn) ufcn->res.fcn = 0;
			break;
		  case MPT_SOLVER_ENUM(NlsJac):
			if (ufcn) ufcn->jac.fcn = 0;
			break;
		  case MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsJac):
			if (ufcn) {
				ufcn->res.fcn = 0;
				ufcn->jac.fcn = 0;
			}
			break;
		  default:
			return MPT_ERROR(BadType);
		}
	}
	else if (ufcn) {
		switch (type) {
		  case MPT_SOLVER_ENUM(NlsVector):
			if (!((MPT_SOLVER_NLS_STRUCT(residuals) *) ptr)->fcn) {
				return MPT_ERROR(BadValue);
			}
			ufcn->res = *((MPT_SOLVER_NLS_STRUCT(residuals) *) ptr);
			break;
		  case MPT_SOLVER_ENUM(NlsJac):
			ufcn->jac = *((MPT_SOLVER_NLS_STRUCT(jacobian) *) ptr);
			break;
		  case MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsJac):
			if (!((MPT_SOLVER_NLS_STRUCT(functions) *) ptr)->res.fcn) {
				return MPT_ERROR(BadValue);
			}
			*ufcn = *((MPT_SOLVER_NLS_STRUCT(functions) *) ptr);
			break;
		  default:
			return MPT_ERROR(BadType);
		}
	}
	if (!(n2->ui = (void *) ufcn)) {
		n2->res.res = 0;
		n2->jac.jac = 0;
		return 0;
	}
	ret = 0;
	if (ufcn->res.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsVector);
		n2->res.res = portn2_fcn;
	} else {
		n2->res.res = 0;
	}
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsJac);
		n2->jac.jac = portn2_jac;
		n2->nd =  0;
	} else {
		n2->jac.jac = 0;
		n2->nd = -1;
	}
	return ret;
}

