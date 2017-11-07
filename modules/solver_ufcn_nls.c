/*!
 * MPT solver module helper function
 *   assign nonlinear solver functions according to parameter
 */

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(ufcn_nls)(long nres, MPT_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if (!ufcn) {
		return nres ? MPT_SOLVER_ENUM(NlsOverdet) : 0;
	}
	if (!ptr) {
		if (!(type & MPT_SOLVER_ENUM(NlsVector))) {
			ufcn->res.fcn = 0;
		}
		if (!(type & MPT_SOLVER_ENUM(NlsJac))) {
			ufcn->jac.fcn = 0;
		}
	}
	else switch (type) {
	  case MPT_SOLVER_ENUM(NlsVector):
		if (!((MPT_NLS_STRUCT(residuals) *) ptr)->fcn) {
			return MPT_ERROR(BadValue);
		}
		ufcn->res = *((MPT_NLS_STRUCT(residuals) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(NlsJac):
		ufcn->jac = *((MPT_NLS_STRUCT(jacobian) *) ptr);
		break;
	  case MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsJac):
		if (!((MPT_NLS_STRUCT(functions) *) ptr)->res.fcn) {
			return MPT_ERROR(BadValue);
		}
		*ufcn = *((MPT_NLS_STRUCT(functions) *) ptr);
		break;
	  default:
		return MPT_ERROR(BadType);
	}
	ret = 0;
	if (ufcn->res.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsVector);
		if (nres) {
			ret |= MPT_SOLVER_ENUM(NlsOverdet);
		}
	}
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsJac);
	}
	return ret;
}
