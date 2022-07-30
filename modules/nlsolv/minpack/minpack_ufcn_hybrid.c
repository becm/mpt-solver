/*!
 * generic user function wrapper for MINPACK HYBRID instance
 */

#include "types.h"

#include "minpack.h"

static void hybrd_fcn(int *neq, double *x, double *f, int *flag)
{
	const MPT_SOLVER_STRUCT(minpack) *mp;
	const MPT_NLS_STRUCT(functions) *uf;
	int flg, ld[2];
	
	mp = (MPT_SOLVER_STRUCT(minpack) *) neq;
	uf = mp->ufcn;
	
	if (!*flag) {
		const MPT_NLS_STRUCT(output) *out;
		if ((out = mp->out) && out->fcn) {
			MPT_STRUCT(value) par, res;
			struct iovec pvec, rvec;
			
			pvec.iov_base = x;
			pvec.iov_len  = mp->nls.nval * sizeof(*x);
			MPT_value_set(&par, MPT_type_toVector('d'), &pvec);
			
			rvec.iov_base = f;
			rvec.iov_len  = mp->nls.nval * sizeof(double);
			MPT_value_set(&res, MPT_type_toVector('d'), &rvec);
			
			out->fcn(out->par, &par, &res);
		}
		return;
	}
	ld[0] = ld[1] = *neq;
	
	if ((flg = uf->res.fcn(uf->res.par, x, f, ld)) < 0) {
		*flag = flg;
	}
}

static void hybrj_fcn(int *neq, double *x, double *f, double *jac, int *ldjac, int *flag)
{
	if (*flag == 2) {
		const MPT_SOLVER_STRUCT(minpack) *mp = (MPT_SOLVER_STRUCT(minpack) *) neq;
		const MPT_NLS_STRUCT(functions) *uf = mp->ufcn;
		int flg, ld[3];
		
		ld[0] = *ldjac;
		ld[1] = ld[2] = *neq;
		
		if ((flg = uf->jac.fcn(uf->jac.par, x, jac, ld, f)) < 0) {
			*flag = flg;
		}
		return;
	}
	hybrd_fcn(neq, x, f, flag);
}

extern int mpt_minpack_ufcn_hybrid(MPT_SOLVER_STRUCT(minpack) *mp)
{
	const MPT_NLS_STRUCT(functions) *ufcn;
	int ret;
	
	if (mp->nls.nval != mp->nls.nres) {
		return MPT_ERROR(BadArgument);
	}
	if (!(ufcn = mp->ufcn) || !ufcn->res.fcn) {
		return MPT_ERROR(BadValue);
	}
	ret = MPT_SOLVER_ENUM(NlsVector);
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsJac);
		mp->fcn.hj = hybrj_fcn;
		mp->solv = MPT_ENUM(MinpackHybrj);
	} else {
		mp->fcn.hd = hybrd_fcn;
		mp->solv = MPT_ENUM(MinpackHybrd);
	}
	return ret;
}

