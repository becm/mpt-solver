/*!
 * generic user function wrapper for MINPACK HYBRID instance
 */

#include "minpack.h"

static void hybrd_fcn(int *neq, double *x, double *f, int *flag)
{
	MPT_SOLVER_STRUCT(minpack) *mp;
	const MPT_SOLVER_NLS_STRUCT(functions) *uf;
	int flg, ld[2];
	
	mp = (MPT_SOLVER_STRUCT(minpack) *) neq;
	uf = mp->ufcn;
	
	if (!*flag) {
		if (uf->out) {
			static const char fmt[] = { MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
			struct iovec vec[2];
			MPT_STRUCT(value) val;
			
			vec[0].iov_base = x;
			vec[0].iov_len  = mp->nls.nval * sizeof(double);
			vec[1].iov_base = f;
			vec[1].iov_len  = mp->nls.nval * sizeof(double);
			
			val.fmt = fmt;
			val.ptr = vec;
			uf->out(uf->opar, &val);
		}
		return;
	}
	ld[0] = ld[1] = *neq;
	
	if ((flg = uf->res(uf->rpar, x, f, ld)) < 0) {
		*flag = flg;
	}
}

static void hybrj_fcn(int *neq, double *x, double *f, double *jac, int *ldjac, int *flag)
{
	if (*flag == 2) {
		MPT_SOLVER_STRUCT(minpack) *mp = (MPT_SOLVER_STRUCT(minpack) *) neq;
		const MPT_SOLVER_NLS_STRUCT(functions) *uf = mp->ufcn;
		int flg, ld[3];
		
		ld[0] = *ldjac;
		ld[1] = ld[2] = *neq;
		
		if ((flg = uf->jac(uf->jpar, x, jac, ld, f)) < 0) {
			*flag = flg;
		}
		return;
	}
	hybrd_fcn(neq, x, f, flag);
}

extern int mpt_minpack_ufcn_hybrid(MPT_SOLVER_STRUCT(minpack) *mp)
{
	if (!mp->ufcn || !mp->ufcn->res) {
		return MPT_ERROR(BadArgument);
	}
	switch (mp->solv) {
	    case 0:
		mp->solv = MPT_ENUM(MinpackHybrj);
	    case MPT_ENUM(MinpackHybrj):
		mp->fcn.hj = hybrj_fcn;
		if (mp->ufcn->jac) break;
		mp->solv = MPT_ENUM(MinpackHybrd);
	    case MPT_ENUM(MinpackHybrd):
		mp->fcn.hd = hybrd_fcn;
		break;
	    default:
		return MPT_ERROR(BadValue);
	}
	return 0;
}

