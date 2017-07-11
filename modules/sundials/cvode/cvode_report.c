/*!
 * report for CVode solver
 */

#include <sys/uio.h>

#include <cvode/cvode_impl.h>

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
extern int sundials_cvode_report(const MPT_SOLVER_STRUCT(cvode) *cv, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	static const char longfmt[] = { 'l', 0 };
	MPT_STRUCT(property) pr;
	long int lval;
	double dval;
	int line = 0;
	CVodeMem cv_mem = cv->mem;
	
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	
	pr.name = "method";
	pr.desc = MPT_tr("method for solver step");
	pr.val.fmt = 0;
	
	switch (cv_mem->cv_lmm) {
	  case CV_ADAMS: pr.val.ptr = "Adams"; break;
	  case CV_BDF:   pr.val.ptr = "BDF";   break;
	  default:       pr.val.ptr = "";
	}
	if (out(usr, &pr) > 0) ++line;
	
	if (sundials_report_jac(&cv->sd, out, usr) >= 0) ++line;
	
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (CVodeGetCurrentTime(cv_mem, &dval) == CV_SUCCESS)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.val.fmt = "d";
	pr.val.ptr = &dval;
	out(usr, &pr);
	++line;
	}
	
	if ((show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)))
	    && (CVodeGetNumSteps(cv_mem, &lval) == CV_SUCCESS)) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	realtype *val = cv->sd.y ? N_VGetArrayPointer(cv->sd.y) : 0;
	MPT_SOLVER_MODULE_FCN(ivp_values)(&cv->ivp, cv->t, val, MPT_tr("CVode solver state"), out, usr);
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (CVodeGetLastStep(cv_mem, &dval) == CV_SUCCESS)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "d";
	pr.val.ptr = &dval;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	if (CVodeGetNumRhsEvals(cv_mem, &lval) == CV_SUCCESS) {
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	if ((cv->sd.linalg & MPT_SOLVER_ENUM(SundialsDls))
	    && (CVodeGetNumLinSolvSetups(cv_mem, &lval) == CV_SUCCESS)) {
	pr.name = "lsetup";
	pr.desc = MPT_tr("linear solver setups");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	if (CVodeGetNumNonlinSolvIters(cv_mem, &lval) == CV_SUCCESS) {
	pr.name = "nliter";
	pr.desc = MPT_tr("nonlinear solver iterations");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	if (CVodeGetNumNonlinSolvConvFails(cv_mem, &lval) == CV_SUCCESS
	    && lval) {
	pr.name = "nlfail";
	pr.desc = MPT_tr("nonlinear solver conv. fail");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	return line;
}

