/*!
 * generic user function wrapper for MINPACK LMDERV instance
 */

#include "minpack.h"

static void lmdif_fcn(int *m, int *n, double *x, double *f, int *flag)
{
	MPT_SOLVER_STRUCT(minpack) *mp;
	const MPT_SOLVER_STRUCT(nlsfcn) *uf;
	int flg, ld[2];
	
	mp = (MPT_SOLVER_STRUCT(minpack) *) n;
	uf = mp->ufcn;
	
	if (!*flag) {
		if (uf->out) {
			static const char fmt[] = { MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
			struct iovec vec[2];
			MPT_STRUCT(value) val;
			
			vec[0].iov_base = x;
			vec[0].iov_len  = mp->nls.nval * sizeof(double);
			vec[1].iov_base = f;
			vec[1].iov_len  = mp->nls.nres * sizeof(double);
			
			val.fmt = fmt;
			val.ptr = vec;
			uf->out(uf->opar, &val);
		}
		return;
	}
	ld[0] = *m;
	ld[1] = *n;
	
	if ((flg = uf->res(uf->rpar, x, f, ld)) < 0) {
		*flag = flg;
	}
}

static void lmder_fcn(int *m, int *n, double *x, double *f, double *jac, int *ldjac, int *flag)
{
	if (*flag == 2) {
		MPT_SOLVER_STRUCT(minpack) *mp = (MPT_SOLVER_STRUCT(minpack) *) n;
		const MPT_SOLVER_STRUCT(nlsfcn) *uf = mp->ufcn;
		int flg, ld[3];
		
		ld[0] = *ldjac;
		ld[1] = *m;
		ld[2] = *n;
		
		if ((flg = uf->jac(uf->jpar, x, jac, ld, f)) < 0) {
			*flag = flg;
		}
		return;
	}
	lmdif_fcn(m, n, x, f, flag);
}

extern int mpt_minpack_ufcn_lmderv(MPT_SOLVER_STRUCT(minpack) *mp)
{
	if (!mp->ufcn || !mp->ufcn->res) {
		return MPT_ERROR(BadArgument);
	}
	switch (mp->solv) {
	    case 0:
	    case MPT_ENUM(MinpackLmDer):
		mp->fcn.der = lmder_fcn;
		if (mp->ufcn->jac) break;
		mp->solv = MPT_ENUM(MinpackLmDif);
	    case MPT_ENUM(MinpackLmDif):
		mp->fcn.dif = lmdif_fcn;
		break;
	    default:
		return MPT_ERROR(BadValue);
	}
	return 0;
}

