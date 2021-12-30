/*!
 * prepare/finalize VODE solver,
 * get status information.
 */

#include "vode.h"

#include "module_functions.h"

extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *vd, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	size_t li = vd->iwork.iov_len / sizeof(int);
	size_t lr = vd->rwork.iov_len / sizeof(double);
	int *iwk = vd->iwork.iov_base;
	double *rwk = vd->rwork.iov_base;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	static const uint8_t fmt_ss[] = "ss";
	static const uint8_t fmt_band[] = "siis";
	struct { const char *type; int32_t ml, mu; const char *jac; } d;
	const char *val[2];
	
	pr.name = "method";
	pr.desc = MPT_tr("method for solver step");
	pr.val.fmt = fmt_ss;
	pr.val.ptr = val;
	
	val[0] = (vd->meth == 2) ? "BDF" : "Adams";
	val[1] = MPT_tr("saved");
	
	if (!vd->miter || vd->jsv < 0) {
		pr.val.fmt = 0;
		pr.val.ptr = val[0];
	}
	out(usr, &pr);
	++line;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("type of jacobian");
	pr.val.fmt = fmt_ss;
	pr.val.ptr = val;
	
	val[0] = "Full";
	val[1] = "user";
	d.type = "Banded";
	if (li > 1) {
		d.ml = iwk[0];
		d.mu = iwk[1];
	} else {
		d.ml = d.mu = -1;
	}
	d.jac = val[1];
	
	switch (vd->miter) {
		case 1:
			if (vd->jac) {
				break;
			}
			/* fall through */
		case 2:
			val[0] = "full";
			val[1] = "numerical";
			break;
		case 3:
			pr.val.fmt = 0;
			pr.val.ptr = "diagonal";
			break;
		case 4:
			pr.val.fmt = fmt_band;
			if (vd->jac) {
				pr.val.ptr = &d;
				break;
			}
			/* fall through */
		case 5:
			d.type = "banded";
			d.jac = "numerical";
			pr.val.fmt = fmt_band;
			pr.val.ptr = &d;
			break;
		default:
			pr.val.fmt = 0;
			pr.val.ptr = "none";
	}
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&vd->ivp, vd->t, vd->y, MPT_tr("dVode solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	if (lr > 12) {
		mpt_solver_module_value_double(&pr.val, &rwk[12]);
	} else {
		mpt_solver_module_value_double(&pr.val, &vd->t);
	}
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
