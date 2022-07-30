/*!
 * output dDASSL solver instance information
 */

#include "dassl.h"

#include "module_functions.h"

extern int mpt_dassl_report(const MPT_SOLVER_STRUCT(dassl) *da, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	int *iwork = da->iwork.iov_base;
	double *rwork = da->rwork.iov_base;
	size_t li = da->iwork.iov_len / sizeof(int);
	size_t lr = da->rwork.iov_len / sizeof(double);
	int line =  0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac;
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	
	jac = da->info[4] ? "user" : "numerical";
	
	if (da->info[5]) {
		MPT_STRUCT(property) val[4] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0], da->info[4] ? "Banded" : "banded");
		val[1].name = "ml";
		val[1].desc = MPT_tr("jacobian lower band size");
		mpt_solver_module_value_int(&val[1], &iwork[0]);
		val[2].name = "mu";
		val[2].desc = MPT_tr("jacobian upper band size");
		mpt_solver_module_value_int(&val[2], &iwork[1]);
		val[3].name = "jac_method";
		val[3].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[3], jac);
		
		mpt_solver_module_report_properties(val, 4, pr.name, pr.desc, out, usr);
	} else {
		MPT_STRUCT(property) val[2] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0], da->info[4] ? "Full" : "full");
		val[1].name = "jac_method";
		val[1].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[1], jac);
		
		mpt_solver_module_report_properties(val, 2, pr.name, pr.desc, out, usr);
	}
	
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&da->ivp, da->t, da->y, MPT_tr("DASSL solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	if (lr > 3) {
		mpt_solver_module_value_double(&pr, &rwork[3]);
	} else {
		mpt_solver_module_value_double(&pr, &da->t);
	}
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status)) && li > 10) {
	pr.name = "n";
	pr.desc = "integration steps";
	mpt_solver_module_value_int(&pr, &iwork[10]);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 2) {
	pr.name = "h";
	pr.desc = "current step size";
	mpt_solver_module_value_double(&pr, &rwork[2]);
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && li > 14) {
	
	pr.name = "reval";
	pr.desc = "residual evaluations";
	mpt_solver_module_value_int(&pr, &iwork[11]);
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = "jacobian evaluations";
	mpt_solver_module_value_int(&pr, &iwork[12]);
	out(usr, &pr);
	++line;
	
	pr.name = "etfail";
	pr.desc = "error test failures";
	mpt_solver_module_value_int(&pr, &iwork[13]);
	out(usr, &pr);
	++line;
	
	pr.name = "cvfail";
	pr.desc = "convergence failures";
	mpt_solver_module_value_int(&pr, &iwork[14]);
	out(usr, &pr);
	++line;
	}
	return line;
}

