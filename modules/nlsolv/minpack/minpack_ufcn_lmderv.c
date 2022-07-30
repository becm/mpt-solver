/*!
 * generic user function wrapper for MINPACK LMDERV instance
 */

#include "types.h"

#include "minpack.h"

static void lmdif_fcn(int *m, int *n, double *x, double *f, int *flag)
{
	const MPT_SOLVER_STRUCT(minpack) *mp;
	const MPT_NLS_STRUCT(functions) *uf;
	int flg, ld[2];
	
	mp = (MPT_SOLVER_STRUCT(minpack) *) n;
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
			rvec.iov_len  = mp->nls.nres * sizeof(double);
			MPT_value_set(&res, MPT_type_toVector('d'), &rvec);
			
			out->fcn(out->par, &par, &res);
		}
		return;
	}
	ld[0] = *m;
	ld[1] = *n;
	
	if ((flg = uf->res.fcn(uf->res.par, x, f, ld)) < 0) {
		*flag = flg;
	}
}

static void lmder_fcn(int *m, int *n, double *x, double *f, double *jac, int *ldjac, int *flag)
{
	if (*flag == 2) {
		const MPT_SOLVER_STRUCT(minpack) *mp = (MPT_SOLVER_STRUCT(minpack) *) n;
		const MPT_NLS_STRUCT(functions) *uf = mp->ufcn;
		int flg, ld[3];
		
		ld[0] = *ldjac;
		ld[1] = *m;
		ld[2] = *n;
		
		if ((flg = uf->jac.fcn(uf->jac.par, x, jac, ld, f)) < 0) {
			*flag = flg;
		}
		return;
	}
	lmdif_fcn(m, n, x, f, flag);
}

extern int mpt_minpack_ufcn_lmderv(MPT_SOLVER_STRUCT(minpack) *mp)
{
	const MPT_NLS_STRUCT(functions) *ufcn;
	int ret;
	
	if (mp->nls.nval > mp->nls.nres) {
		return MPT_ERROR(BadArgument);
	}
	if (!(ufcn = mp->ufcn) || !ufcn->res.fcn) {
		return MPT_ERROR(BadValue);
	}
	ret = MPT_SOLVER_ENUM(NlsUser);
	if (ufcn->jac.fcn) {
		ret |= MPT_SOLVER_ENUM(NlsJac);
		mp->fcn.der = lmder_fcn;
		mp->solv = MPT_ENUM(MinpackLmDer);
	} else {
		mp->fcn.dif = lmdif_fcn;
		mp->solv = MPT_ENUM(MinpackLmDif);
	}
	return ret;
}

