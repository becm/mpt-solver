/*!
 * get LIMEX status information.
 */

#include "limex.h"

#include "module_functions.h"

extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *lx, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *jac, *val; int ml, mu; } d;
	int neqs;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	
	neqs = lx->ivp.neqs * (lx->ivp.pint + 1);
	
	d.val = lx->jac ? "(user)" : "(numerical)";
	
	d.ml = lx->iopt[7];
	d.mu = lx->iopt[8];
	
	if (d.ml >= 0 && d.ml < neqs) {
		static const uint8_t fmt[] = "ssii";
		pr.val.fmt = fmt;
		d.jac = "banded";
	} else {
		static const uint8_t fmt[] = "ss";
		pr.val.fmt = fmt;
		d.jac = "full";
	}
	pr.val.ptr = &d;
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&lx->ivp, lx->t, lx->y, MPT_tr("dVode solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	mpt_solver_module_value_double(&pr.val, &lx->t);
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = "integration steps";
	mpt_solver_module_value_int(&pr.val, &lx->iopt[27]);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	mpt_solver_module_value_double(&pr.val, &lx->h);
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) {
		return line;
	}
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	mpt_solver_module_value_int(&pr.val, &lx->iopt[23]);
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	mpt_solver_module_value_int(&pr.val, &lx->iopt[28]);
	out(usr, &pr);
	++line;
	
	pr.name = "jfeval";
	pr.desc = "jacobian f eval.";
	mpt_solver_module_value_int(&pr.val, &lx->iopt[24]);
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	mpt_solver_module_value_int(&pr.val, &lx->iopt[25]);
	out(usr, &pr);
	++line;
	
	pr.name = "lubsub";
	pr.desc = MPT_tr("LU back-subst.");
	mpt_solver_module_value_int(&pr.val, &lx->iopt[26]);
	out(usr, &pr);
	++line;
	
	return line;
}
