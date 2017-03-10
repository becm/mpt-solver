/*!
 * prepare/finalize VODE solver,
 * get status information.
 */

#include "vode.h"

extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *vd, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	size_t li = vd->iwork.iov_len / sizeof(int);
	size_t lr = vd->rwork.iov_len / sizeof(double);
	int *iwk = vd->iwork.iov_base;
	double *rwk = vd->rwork.iov_base;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *jac, *val; int ml, mu; } d;
	
	pr.name = "method";
	pr.desc = MPT_tr("method for solver step");
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	d.jac = (vd->meth == 2) ? "BDF" : "Adams";
	d.val = MPT_tr("(saved)");
	
	if (!vd->miter || vd->jsv < 0) pr.val.fmt = "s";
	
	out(usr, &pr);
	++line;
	
	d.jac  = "full";
	d.val  = "(user)";
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("type of jacobian");
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	switch (vd->miter) {
		case 1: if (vd->jac) break;
		case 2: d.val = "(numerical)"; break;
		case 3: d.jac = "diagonal"; pr.val.fmt = "s"; break;
		case 4: d.jac = "banded"; pr.val.fmt = "ssii"; d.ml = iwk[0]; d.mu = iwk[1];
			if (vd->jac) break;
		case 5: d.jac = "banded"; d.val = "(numerical)"; pr.val.fmt = "ssii";  d.ml = iwk[0]; d.mu = iwk[1]; break;
		default: d.jac = "none"; pr.val.fmt = "s";
	}
	d.jac = MPT_tr(d.jac);
	d.val = MPT_tr(d.val);
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
		static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
		struct {
			double t;
			struct iovec vec;
		} dat;
		size_t len = vd->ivp.pint + 1;
		
		dat.t = vd->t;
		dat.vec.iov_base = vd->y;
		dat.vec.iov_len  = len * vd->ivp.neqs * sizeof(double);
		
		pr.name = 0;
		pr.desc = MPT_tr("dVode solver state");
		pr.val.fmt = fmt;
		pr.val.ptr = &dat;
		out(usr, &pr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 12) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.val.fmt = "d";
	pr.val.ptr = &rwk[12];
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && li > 10) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[10];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 10) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "d";
	pr.val.ptr = &rwk[10];
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && li > 11) {
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[11];
	out(usr, &pr);
	++line;
	
	if (li > 12 && iwk[12]) {
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[12];
	out(usr, &pr);
	++line;
	}
	
	if (li > 18 && iwk[18]) {
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[18];
	out(usr, &pr);
	++line;
	}
	
	if (li > 19 && iwk[19]) {
	pr.name = "niter";
	pr.desc = MPT_tr("nonlinear (Newton) iterations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[19];
	out(usr, &pr);
	++line;
	}
	}
	return line;
}
