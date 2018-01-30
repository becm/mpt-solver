/*!
 * output dDASSL solver instance information
 */

#include "dassl.h"

#include "module_functions.h"

extern int mpt_dassl_report(const MPT_SOLVER_STRUCT(dassl) *da, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int *iwork = da->iwork.iov_base;
	double *rwork = da->rwork.iov_base;
	size_t li = da->iwork.iov_len / sizeof(int);
	size_t lr = da->rwork.iov_len / sizeof(double);
	int line =  0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac;
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	
	jac = da->info[4] ? "user" : "numerical";
	
	if (da->info[5]) {
		static const uint8_t fmt[] = "siis";
		struct { const char *fmt; int32_t ml, mu; const char *jac; } val;
		
		val.fmt = da->info[4] ? "Banded" : "banded";
		val.ml  = iwork[0];
		val.mu  = iwork[1];
		val.fmt = jac;
		
		pr.val.fmt = fmt;
		pr.val.ptr = &val;
		out(usr, &pr);
	} else {
		static const uint8_t fmt[] = "ss";
		const char *val[2];
		
		val[0] = da->info[4] ? "Full" : "full";
		val[1] = jac;
		
		pr.val.fmt = fmt;
		pr.val.ptr = val;
		out(usr, &pr);
	}
	
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&da->ivp, da->t, da->y, MPT_tr("DASSL solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	if (lr > 3) {
		mpt_solver_module_value_double(&pr.val, &rwork[3]);
	} else {
		mpt_solver_module_value_double(&pr.val, &da->t);
	}
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status)) && li > 10) {
	pr.name = "n";
	pr.desc = "integration steps";
	mpt_solver_module_value_int(&pr.val, &iwork[10]);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 2) {
	pr.name = "h";
	pr.desc = "current step size";
	mpt_solver_module_value_double(&pr.val, &rwork[2]);
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && li > 14) {
	
	pr.name = "reval";
	pr.desc = "residual evaluations";
	mpt_solver_module_value_int(&pr.val, &iwork[11]);
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = "jacobian evaluations";
	mpt_solver_module_value_int(&pr.val, &iwork[12]);
	out(usr, &pr);
	++line;
	
	pr.name = "etfail";
	pr.desc = "error test failures";
	mpt_solver_module_value_int(&pr.val, &iwork[13]);
	out(usr, &pr);
	++line;
	
	pr.name = "cvfail";
	pr.desc = "convergence failures";
	mpt_solver_module_value_int(&pr.val, &iwork[14]);
	out(usr, &pr);
	++line;
	}
	return line;
}

