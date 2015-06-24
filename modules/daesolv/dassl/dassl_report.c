/*!
 * output dDASSL solver instance information
 */

#include "dassl.h"

extern int mpt_dassl_report(const MPT_SOLVER_STRUCT(dassl) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int *iwork = data->iwork.iov_base;
	double *rwork = data->rwork.iov_base;
	int line =  0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *jac, *val; int ml, mu; } d;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	d.jac = "full";
	d.val = data->info[4] ? "(user)" : "(numerical)";
	if (data->info[5]) { d.jac = "banded"; pr.val.fmt = "ssii"; }
	
	d.ml = iwork[0];
	d.mu = iwork[1];
	
	out(usr, &pr);
	++line;
	}
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "D";
	pr.val.ptr = &rwork[3];
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status))) {
	pr.name = "n";
	pr.desc = "integration steps";
	pr.val.fmt = "i";
	pr.val.ptr = &iwork[10];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	pr.val.fmt = "D";
	pr.val.ptr = &rwork[2];
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report))) {
	
	pr.name = "reval";
	pr.desc = "residual evaluations";
	pr.val.fmt = "i";
	pr.val.ptr = &iwork[11];
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = "jacobian evaluations";
	pr.val.fmt = "i";
	pr.val.ptr = &iwork[12];
	out(usr, &pr);
	++line;
	
	pr.name = "etfail";
	pr.desc = "error test failures";
	pr.val.fmt = "i";
	pr.val.ptr = &iwork[13];
	out(usr, &pr);
	++line;
	
	pr.name = "cvfail";
	pr.desc = "convergence failures";
	pr.val.fmt = "i";
	pr.val.ptr = &iwork[14];
	out(usr, &pr);
	++line;
	}
	return line;
}

