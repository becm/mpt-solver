/*!
 * prepare/finalize VODE solver,
 * get status information.
 */

#include <errno.h>

#include "vode.h"

extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int *iwk = data->iwork.iov_base;
	double *rwk = data->rwork.iov_base;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *jac, *val; int ml, mu; } d;
	
	pr.name = "method";
	pr.desc = MPT_tr("method for solver step");
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	d.jac = (data->meth == 2) ? "BDF" : "Adams";
	d.val = MPT_tr("(saved)");
	
	if (!data->miter || data->jsv < 0) pr.val.fmt = "s";
	
	out(usr, &pr);
	++line;
	
	d.ml = iwk[0];
	d.mu = iwk[1];
	
	d.jac  = "full";
	d.val  = "(user)";
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("type of jacobian");
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	switch (data->miter) {
		case 1: if (data->jac) break;
		case 2: d.val = "(numerical)"; break;
		case 3: d.jac = "diagonal"; pr.val.fmt = "s"; break;
		case 4: d.jac = "banded"; pr.val.fmt = "ssii";
			if (data->jac) break;
		case 5: d.jac = "banded"; d.val = "(numerical)"; pr.val.fmt = "ssii"; break;
		default: d.jac = "none"; pr.val.fmt = "s";
	}
	d.jac = MPT_tr(d.jac);
	d.val = MPT_tr(d.val);
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.val.fmt = "d";
	pr.val.ptr = &rwk[12];
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[10];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "d";
	pr.val.ptr = &rwk[10];
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report))) {
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[11];
	out(usr, &pr);
	++line;
	
	if (iwk[12]) {
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[12];
	out(usr, &pr);
	++line;
	}
	
	if (iwk[18]) {
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &iwk[18];
	out(usr, &pr);
	++line;
	}
	
	if (iwk[19]) {
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
