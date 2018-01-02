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
	int line =  0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *jac, *val; int32_t ml, mu; } d;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.ptr = &d;
	
	d.jac = "full";
	d.val = da->info[4] ? "(user)" : "(numerical)";
	if (!da->info[5]) {
		static const uint8_t fmt[] = "ss";
		pr.val.fmt = fmt;
	} else {
		static const uint8_t fmt[] = "ssii";
		pr.val.fmt = fmt;
		d.jac = "banded";
		d.ml = iwork[0];
		d.mu = iwork[1];
	}
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&da->ivp, da->t, da->y, MPT_tr("DASSL solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && rwork) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	mpt_solver_module_value_double(&pr.val, &rwork[3]);
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status)) && iwork) {
	pr.name = "n";
	pr.desc = "integration steps";
	mpt_solver_module_value_int(&pr.val, &iwork[10]);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && rwork) {
	pr.name = "h";
	pr.desc = "current step size";
	mpt_solver_module_value_double(&pr.val, &rwork[2]);
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && iwork) {
	
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

