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
	const char *jac;
	int neqs;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	jac = lx->jac ? "user" : "numerical";
	
	neqs = lx->ivp.neqs * (lx->ivp.pint + 1);
	
	if (lx->iopt[7] >= 0 && lx->iopt[7] < neqs) {
		static const uint8_t fmt[] = "siis";
		struct { const char *fmt; int32_t ml, mu; const char *jac; } val;
		
		val.fmt = lx->jac ? "Banded" : "banded";
		val.ml  = lx->iopt[7];
		val.mu  = lx->iopt[8];
		val.jac = jac;
		
		pr.val.fmt = fmt;
		pr.val.ptr = &val;
		out(usr, &pr);
	} else {
		static const uint8_t fmt[] = "ss";
		const char *val[2];
		
		val[0] = lx->jac ? "Full" : "full";
		val[1] = jac;
		
		pr.val.fmt = fmt;
		pr.val.ptr = val;
		out(usr, &pr);
	}
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&lx->ivp, lx->t, lx->y, MPT_tr("LIMEX solver state"), out, usr);
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
