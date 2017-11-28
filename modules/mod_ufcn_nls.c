/*!
 * MPT solver module helper function
 *   assign nonlinear solver functions according to parameter
 */

#include "../solver.h"

static int fcnFlags(const MPT_NLS_STRUCT(parameters) *par, const MPT_NLS_STRUCT(functions) *ufcn)
{
	int ret = 0;
	
	if (ufcn->res.fcn) {
		if (!par->nres) {
			ret |= MPT_SOLVER_ENUM(NlsOverdet);
		}
		else if (par->nres > par->nval) {
			ret |= MPT_SOLVER_ENUM(NlsOverdet) | MPT_SOLVER_ENUM(NlsVector);
		}
		else {
			ret |= MPT_SOLVER_ENUM(NlsVector);
		}
	}
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsJac);
	}
	return ret;
}

extern int mpt_solver_module_ufcn_nls(const MPT_NLS_STRUCT(parameters) *par, MPT_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	if (!ufcn) {
		return par->nres ? MPT_SOLVER_ENUM(NlsOverdet) : 0;
	}
	if (!ptr) {
		if ((type & MPT_SOLVER_ENUM(NlsVector))
		    || (type & MPT_SOLVER_ENUM(NlsOverdet))) {
			ufcn->res.fcn = 0;
		}
		if ((type & MPT_SOLVER_ENUM(NlsJac))) {
			ufcn->jac.fcn = 0;
		}
		return fcnFlags(par, ufcn);
	}
	switch (type) {
	  case MPT_SOLVER_ENUM(NlsVector):
		if (par->nres != par->nval) {
			return MPT_ERROR(BadArgument);
		}
		break;
	  case MPT_SOLVER_ENUM(NlsOverdet):
		if (par->nres) {
			return MPT_ERROR(BadArgument);
		}
		break;
	  case MPT_SOLVER_ENUM(NlsVector) | MPT_SOLVER_ENUM(NlsOverdet):
		if (par->nres <= par->nval) {
			return MPT_ERROR(BadArgument);
		}
		break;
	  case MPT_SOLVER_ENUM(NlsJac):
		ufcn->jac = *((MPT_NLS_STRUCT(jacobian) *) ptr);
		return fcnFlags(par, ufcn);
	  case MPT_SOLVER_ENUM(NlsVector)  | MPT_SOLVER_ENUM(NlsJac):
	  case MPT_SOLVER_ENUM(NlsOverdet) | MPT_SOLVER_ENUM(NlsJac):
	  case MPT_SOLVER_ENUM(NlsUser)    | MPT_SOLVER_ENUM(NlsJac):
		if (!((MPT_NLS_STRUCT(functions) *) ptr)->res.fcn) {
			return MPT_ERROR(BadValue);
		}
		*ufcn = *((MPT_NLS_STRUCT(functions) *) ptr);
		return fcnFlags(par, ufcn);
	  default:
		return MPT_ERROR(BadType);
	}
	if (!((MPT_NLS_STRUCT(residuals) *) ptr)->fcn) {
		return MPT_ERROR(BadValue);
	}
	ufcn->res = *((MPT_NLS_STRUCT(residuals) *) ptr);
	return fcnFlags(par, ufcn);
}
