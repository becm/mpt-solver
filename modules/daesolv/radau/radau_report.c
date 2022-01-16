/*!
 * output extended information for radau.
 */

#include "radau.h"

#include "module_functions.h"

extern int mpt_radau_report(const MPT_SOLVER_STRUCT(radau) *rd, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	int val;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac;
	pr.name = "jacobian";
	pr.desc = MPT_tr("method for jacobian");
	
	val = rd->ivp.neqs * (rd->ivp.pint + 1);
	
	jac = rd->jac ? "user" : "numerical";
	
	if (rd->mljac >= 0 && rd->mljac < val) {
		MPT_STRUCT(property) val[4] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0].val, rd->jac ? "Banded" : "banded");
		val[1].name = "ml";
		val[1].desc = MPT_tr("jacobian lower band size");
		mpt_solver_module_value_int(&val[1].val, &rd->mljac);
		val[2].name = "mu";
		val[2].desc = MPT_tr("jacobian upper band size");
		mpt_solver_module_value_int(&val[2].val, &rd->mujac);
		val[3].name = "jac_method";
		val[3].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[3].val, jac);
		
		mpt_solver_module_report_properties(val, 4, pr.name, pr.desc, out, usr);
	} else {
		MPT_STRUCT(property) val[2] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0].val, rd->jac ? "Full" : "full");
		val[1].name = "jac_method";
		val[1].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[1].val, jac);
		
		mpt_solver_module_report_properties(val, 2, pr.name, pr.desc, out, usr);
	}
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&rd->ivp, rd->t, rd->y, MPT_tr("RADAU solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	mpt_solver_module_value_double(&pr.val, &rd->t);
	out(usr, &pr);
	++line;
	}
	if ((show & MPT_SOLVER_ENUM(Report)) || (show & MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = "integration steps";
	val = rd->count.st.nsteps;
	mpt_solver_module_value_int(&pr.val, &val);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	mpt_solver_module_value_double(&pr.val, &rd->h);
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) {
		return line;
	}
	
	if (rd->count.st.nsteps != rd->count.st.naccpt) {
		pr.name = "nacc";
		pr.desc = MPT_tr("accepted steps");
		val = rd->count.st.naccpt;
		mpt_solver_module_value_int(&pr.val, &val);
		out(usr, &pr);
		++line;
	}
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	val = rd->count.st.nfev;
	mpt_solver_module_value_int(&pr.val, &val); 
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	val = rd->count.st.njac;
	mpt_solver_module_value_int(&pr.val, &val); 
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	val = rd->count.st.nlud;
	mpt_solver_module_value_int(&pr.val, &val); 
	out(usr, &pr);
	++line;
	
	return line;
}

