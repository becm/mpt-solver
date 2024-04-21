/*!
 * report for CVode solver
 */

#include <sys/uio.h>

#include <cvode/cvode.h>

#include "sundials.h"

#include "module_functions.h"

/*!
 * \ingroup mptSundialsCVode
 * \brief CVode solver report
 * 
 * Process CVode report information.
 * Report properties are emitted to passed handler.
 * 
 * \param data  CVode solver data
 * \param show  types of report entries
 * \param out   handler for report properties
 * \param usr   data context for handler
 * 
 * \return number of reported properties
 */
extern int mpt_sundials_cvode_report(const MPT_SOLVER_STRUCT(cvode) *cv, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	const char *sval;
	long int lval;
	double dval;
	int line = 0;
	void *cv_mem = cv->mem;
	
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	
	pr.name = "method";
	pr.desc = MPT_tr("method for solver step");
	
	switch (cv->method) {
	  case CV_ADAMS: sval = "Adams"; break;
	  case CV_BDF:   sval = "BDF";   break;
	  default:       sval = "";
	}
	mpt_solver_module_value_string(&pr, sval);
	if (out(usr, &pr) > 0) ++line;
	
	if (mpt_sundials_report_jac(&cv->sd, out, usr) >= 0) {
		++line;
	}
	}
	
	if (!(cv_mem = cv->mem)) {
		return line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	sunrealtype *val = cv->sd.y ? N_VGetArrayPointer(cv->sd.y) : 0;
	MPT_SOLVER_MODULE_FCN(ivp_values)(&cv->ivp, cv->t, val, MPT_tr("CVode solver state"), out, usr);
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (CVodeGetCurrentTime(cv_mem, &dval) == CV_SUCCESS)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	mpt_solver_module_value_double(&pr, &dval);
	out(usr, &pr);
	++line;
	}
	
	if ((show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)))
	    && (CVodeGetNumSteps(cv_mem, &lval) == CV_SUCCESS)) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	mpt_solver_module_value_long(&pr, &lval);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	dval = cv->step.hin;  /* use initial value if no solver steps were performed */
	if (!lval || (CVodeGetLastStep(cv_mem, &dval) == CV_SUCCESS)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	mpt_solver_module_value_double(&pr, &dval);
	out(usr, &pr);
	++line;
	}
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	if (CVodeGetNumRhsEvals(cv_mem, &lval) == CV_SUCCESS) {
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	mpt_solver_module_value_long(&pr, &lval);
	out(usr, &pr);
	++line;
	}
	
	if ((cv->sd.linsol >= MPT_SOLVER_SUNDIALS(Direct))
	    && (CVodeGetNumLinSolvSetups(cv_mem, &lval) == CV_SUCCESS)) {
	pr.name = "lsetup";
	pr.desc = MPT_tr("linear solver setups");
	mpt_solver_module_value_long(&pr, &lval);
	out(usr, &pr);
	++line;
	}
	
	if (CVodeGetNumNonlinSolvIters(cv_mem, &lval) == CV_SUCCESS) {
	pr.name = "nliter";
	pr.desc = MPT_tr("nonlinear solver iterations");
	mpt_solver_module_value_long(&pr, &lval);
	out(usr, &pr);
	++line;
	}
	
	if (CVodeGetNumNonlinSolvConvFails(cv_mem, &lval) == CV_SUCCESS
	    && lval) {
	pr.name = "nlfail";
	pr.desc = MPT_tr("nonlinear solver conv. fail");
	mpt_solver_module_value_long(&pr, &lval);
	out(usr, &pr);
	++line;
	}
	
	return line;
}

