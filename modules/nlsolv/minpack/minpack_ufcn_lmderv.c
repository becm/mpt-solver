/*!
 * generic user function wrapper for MINPACK LMDERV instance
 */

#include "minpack.h"

static void lmdif_fcn(int *m, int *n, double *x, double *f, int *flag)
{
	const MPT_SOLVER_STRUCT(minpack) *mp;
	const MPT_SOLVER_NLS_STRUCT(functions) *uf;
	int flg, ld[2];
	
	mp = (MPT_SOLVER_STRUCT(minpack) *) n;
	uf = mp->ufcn;
	
	if (!*flag) {
		const MPT_SOLVER_NLS_STRUCT(output) *out;
		if ((out = mp->out) && out->fcn) {
			static const char fmt[] = { MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
			struct iovec vec[2];
			MPT_STRUCT(value) val;
			
			vec[0].iov_base = x;
			vec[0].iov_len  = mp->nls.nval * sizeof(double);
			vec[1].iov_base = f;
			vec[1].iov_len  = mp->nls.nres * sizeof(double);
			
			val.fmt = fmt;
			val.ptr = vec;
			out->fcn(out->par, &val);
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
		const MPT_SOLVER_NLS_STRUCT(functions) *uf = mp->ufcn;
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

extern int mpt_minpack_ufcn_lmderv(MPT_SOLVER_STRUCT(minpack) *mp, MPT_SOLVER_NLS_STRUCT(functions) *ufcn, int type, const void *ptr)
{
	int ret;
	
	if (mp->nls.nval > mp->nls.nres) {
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
		mp->fcn.der = 0;
		mp->solv = 0;
		return 0;
	}
	ret = MPT_SOLVER_ENUM(NlsVector);
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

