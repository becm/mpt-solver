/*!
 * generic user function wrapper for MINPACK HYBRID instance
 */

#include "minpack.h"

extern int mpt_minpack_ufcn(MPT_SOLVER_STRUCT(minpack) *mp, MPT_SOLVER_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if (mp->nls.nval != mp->nls.nres) {
		return MPT_ERROR(BadArgument);
	}
	if (!ptr) {
		switch (type) {
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
	if (!(mp->ufcn = ufcn) || !ufcn->res.fcn) {
		mp->solv = 0;
		mp->fcn.hd = 0;
		mp->solv = 0;
		return 0;
	}
	if (mp->nls.nres != mp->nls.nval) {
		return mpt_minpack_ufcn_lmderv(mp);
	}
	return mpt_minpack_ufcn_hybrid(mp);
}
