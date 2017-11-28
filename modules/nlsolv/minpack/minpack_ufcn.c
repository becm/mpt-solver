/*!
 * generic user function wrapper for MINPACK HYBRID instance
 */

#include "minpack.h"

#include "module_functions.h"

extern int mpt_minpack_ufcn(MPT_SOLVER_STRUCT(minpack) *mp, MPT_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if (mp->nls.nval > mp->nls.nres) {
		return MPT_ERROR(BadArgument);
	}
	if ((ret = mpt_solver_module_ufcn_nls(&mp->nls, ufcn, type, ptr)) < 0) {
		return ret;
	}
	if (!(mp->ufcn = ufcn) || !ufcn->res.fcn) {
		mp->solv = 0;
		mp->fcn.hd = 0;
		mp->solv = 0;
		return 0;
	}
	if (mp->nls.nval < mp->nls.nres) {
		return mpt_minpack_ufcn_lmderv(mp);
	}
	return mpt_minpack_ufcn_hybrid(mp);
}
