/*!
 * output dDASSL solver instance information
 */

#include "dassl.h"

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
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	d.jac = "full";
	d.val = da->info[4] ? "(user)" : "(numerical)";
	if (da->info[5]) {
		d.jac = "banded";
		d.ml = iwork[0];
		d.mu = iwork[1];
		pr.val.fmt = "ssii";
	}
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
	struct {
		double t;
		struct iovec vec;
	} dat;
	size_t len = da->ivp.pint + 1;
	
	dat.t = da->t;
	dat.vec.iov_base = da->y;
	dat.vec.iov_len  = len * da->ivp.neqs * sizeof(double);
	
	pr.name = 0;
	pr.desc = MPT_tr("LIMEX solver state");
	pr.val.fmt = fmt;
	pr.val.ptr = &dat;
	out(usr, &pr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && rwork) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "d";
	pr.val.ptr = &rwork[3];
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status)) && iwork) {
	pr.name = "n";
	pr.desc = "integration steps";
	pr.val.fmt = "i";
	pr.val.ptr = &iwork[10];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && rwork) {
	pr.name = "h";
	pr.desc = "current step size";
	pr.val.fmt = "d";
	pr.val.ptr = &rwork[2];
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && iwork) {
	
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

