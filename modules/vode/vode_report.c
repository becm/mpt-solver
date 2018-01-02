/*!
 * prepare/finalize VODE solver,
 * get status information.
 */

#include "vode.h"

#include "module_functions.h"

extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *vd, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	static const uint8_t fmt_ss[] = "ss";
	static const uint8_t fmt_band[] = "ssii";
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
	pr.val.fmt = fmt_ss;
	pr.val.ptr = &d;
	
	d.jac = (vd->meth == 2) ? "BDF" : "Adams";
	d.val = MPT_tr("(saved)");
	
	if (!vd->miter || vd->jsv < 0) pr.val.fmt = fmt_ss + 1;
	
	out(usr, &pr);
	++line;
	
	d.jac  = "full";
	d.val  = "(user)";
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("type of jacobian");
	pr.val.fmt = fmt_ss;
	pr.val.ptr = &d;
	
	switch (vd->miter) {
		case 1: if (vd->jac) break;
		case 2: d.val = "(numerical)"; break;
		case 3: d.jac = "diagonal"; pr.val.fmt = fmt_ss + 1; break;
		case 4: d.jac = "banded"; pr.val.fmt = fmt_band; d.ml = iwk[0]; d.mu = iwk[1];
			if (vd->jac) break;
		case 5: d.jac = "banded"; d.val = "(numerical)"; pr.val.fmt = fmt_band;  d.ml = iwk[0]; d.mu = iwk[1]; break;
		default: d.jac = "none"; pr.val.fmt = fmt_ss + 1;
	}
	d.jac = MPT_tr(d.jac);
	d.val = MPT_tr(d.val);
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&vd->ivp, vd->t, vd->y, MPT_tr("dVode solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 12) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	mpt_solver_module_value_double(&pr.val, &rwk[12]);
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && li > 10) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	mpt_solver_module_value_int(&pr.val, &iwk[10]);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 10) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	mpt_solver_module_value_double(&pr.val, &rwk[10]);
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && li > 11) {
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	mpt_solver_module_value_int(&pr.val, &iwk[11]);
	out(usr, &pr);
	++line;
	
	if (li > 12 && iwk[12]) {
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	mpt_solver_module_value_int(&pr.val, &iwk[12]);
	out(usr, &pr);
	++line;
	}
	
	if (li > 18 && iwk[18]) {
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	mpt_solver_module_value_int(&pr.val, &iwk[18]);
	out(usr, &pr);
	++line;
	}
	
	if (li > 19 && iwk[19]) {
	pr.name = "niter";
	pr.desc = MPT_tr("nonlinear (Newton) iterations");
	mpt_solver_module_value_int(&pr.val, &iwk[19]);
	out(usr, &pr);
	++line;
	}
	}
	return line;
}
