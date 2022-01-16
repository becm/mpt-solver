/*!
 * get LIMEX status information.
 */

#include "limex.h"

#include "module_functions.h"

extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *lx, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac;
	int neqs;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	jac = lx->jac ? "user" : "numerical";
	
	neqs = lx->ivp.neqs * (lx->ivp.pint + 1);
	
	if (lx->iopt[7] >= 0 && lx->iopt[7] < neqs) {
		MPT_STRUCT(property) val[4] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0].val, lx->jac ? "Banded" : "banded");
		val[1].name = "ml";
		val[1].desc = MPT_tr("jacobian lower band size");
		mpt_solver_module_value_int(&val[1].val, &lx->iopt[7]);
		val[2].name = "mu";
		val[2].desc = MPT_tr("jacobian upper band size");
		mpt_solver_module_value_int(&val[2].val, &lx->iopt[8]);
		val[3].name = "jac_method";
		val[3].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[3].val, jac);
		
		mpt_solver_module_report_properties(val, 4, pr.name, pr.desc, out, usr);
	} else {
		MPT_STRUCT(property) val[2] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0].val, lx->jac ? "Full" : "full");
		val[1].name = "jac_method";
		val[1].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[1].val, jac);
		
		mpt_solver_module_report_properties(val, 2, pr.name, pr.desc, out, usr);
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
