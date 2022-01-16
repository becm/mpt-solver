/*!
 * status information for MINPACK instance
 */

#include "types.h"

#include "minpack.h"

extern int mpt_minpack_report(const MPT_SOLVER_STRUCT(minpack) *mpack, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
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
	mpt_solver_module_value_string(&pr.val, solv);
	out(usr, &pr);
	++line;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("jacobian matrix type");
	mpt_solver_module_value_string(&pr.val, jac);
	out(usr, &pr);
	++line;
	}
	if ((show & MPT_SOLVER_ENUM(Values)) && mpack->info >= 0) {
		MPT_STRUCT(property) arg[2] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		const char *desc = MPT_tr("current parameters and residuals");
		struct iovec *vec;
		
		arg[0].name = 0;
		arg[0].desc = MPT_tr("current parameter values");
		arg[0].val.type = MPT_type_toVector('d');
		arg[0].val.ptr = arg[0].val._buf;
		vec = (void *) arg[0].val._buf;
			
		vec->iov_base = mpack->val.iov_base;
		vec->iov_len  = mpack->nls.nval * sizeof(double);
		
		if (mpack->info) {
			arg[0].name = "parameters";
			
			arg[1].name = "residuals";
			arg[1].desc = MPT_tr("residuals for current parameters");
			arg[1].val.type = MPT_type_toVector('d');
			arg[1].val.ptr = arg[1].val._buf;
			vec = (void *) arg[1].val._buf;
			
			vec->iov_base = ((double *) mpack->val.iov_base) + mpack->nls.nval;
			vec->iov_len  = mpack->nls.nres * sizeof(double);
			
			mpt_solver_module_report_properties(arg, 2, 0, desc, out, usr); 
		}
		else {
			out(usr, &arg[0]);
		}
	}
	if (!(show & MPT_SOLVER_ENUM(Report))) {
		return line;
	}
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	mpt_solver_module_value_int(&pr.val, &mpack->nfev);
	out(usr, &pr);
	++line;
	
	if (mpack->njev) {
		pr.name = "jeval";
		pr.desc = MPT_tr("jacobian evaluations");
		mpt_solver_module_value_int(&pr.val, &mpack->njev);
		out(usr, &pr);
		++line;
	}
	return line;
}
