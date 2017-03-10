/*!
 * status information for MINPACK instance
 */

#include "minpack.h"

extern int mpt_minpack_report(const MPT_SOLVER_STRUCT(minpack) *mpack, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	static const char lm[] = "levenberg-marquardt";
	const char *solv = "powell hybrid", *jac = "numeric";
	
	switch (mpack->solv) {
	    case 0:
		if (mpack->nls.nres != mpack->nls.nval) {
			solv = lm;
		}
		break;
	    case MPT_ENUM(MinpackHybrd):
		if (mpack->mu < mpack->nls.nres || mpack->ml < mpack->nls.nres) {
			jac = "banded";
		}
		break;
	    case MPT_ENUM(MinpackHybrj): jac = "user"; break;
	    case MPT_ENUM(MinpackLmDif): solv = lm; break;
	    case MPT_ENUM(MinpackLmDer): solv = lm; jac = "user"; break;
	    case MPT_ENUM(MinpackLmStr): solv = lm; jac = "columnwise user"; break;
	    default:
		return MPT_ERROR(BadArgument);
	}
	pr.name = "method";
	pr.desc = MPT_tr("minpack solver type");
	pr.val.fmt = 0;
	pr.val.ptr = solv;
	out(usr, &pr);
	++line;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("jacobian matrix type");
	pr.val.fmt = 0;
	pr.val.ptr = jac;
	out(usr, &pr);
	++line;
	}
	if ((show & MPT_SOLVER_ENUM(Values)) && mpack->info >= 0) {
		static const char fmt[] = { MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
		struct {
			struct iovec x, f;
		} d;
		
		d.x.iov_base = mpack->val.iov_base;
		d.x.iov_len  = mpack->nls.nval * sizeof(double);
		d.f.iov_base = ((double *) d.x.iov_base) + mpack->nls.nval;
		d.f.iov_len  = mpack->nls.nres * sizeof(double);
		
		pr.name = 0;
		pr.desc = MPT_tr("parameters and residual data");
		pr.val.fmt = mpack->info ? fmt : fmt+1;
		pr.val.ptr = &d;
		
		out(usr, &pr);
	}
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &mpack->nfev;
	out(usr, &pr);
	
	if (!mpack->njev) return line + 1;
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &mpack->njev;
	out(usr, &pr);
	return line + 2;
}
