/*!
 * output extended information for radau.
 */

#include "radau.h"

#include "module_functions.h"

extern int mpt_radau_report(const MPT_SOLVER_STRUCT(radau) *rd, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int val;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *type; int32_t ml, mu; } miter;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("method for jacobian");
	
	val = rd->ivp.neqs * (rd->ivp.pint + 1);
	
	if ((miter.ml = rd->mljac) >= 0 && miter.ml < val) {
		static const uint8_t fmt[] = "sii";
		pr.val.fmt = fmt;
		miter.mu   = rd->mujac;
		miter.type = rd->jac ? "banded(user)" : "banded";
	}
	else {
		static const uint8_t fmt[] = "s";
		pr.val.fmt = fmt;
		miter.type = rd->jac ? "full(user)" : "full";
	}
	pr.val.ptr = &miter;
	
	out(usr, &pr);
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

