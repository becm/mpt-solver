/*!
 * report for CVode solver
 */

#include <errno.h>

#include <cvode/cvode_impl.h>

#include "sundials.h"

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
extern int sundials_cvode_report(const MPT_SOLVER_STRUCT(cvode) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	long lval;
	double dval;
	int64_t val;
	int line = 0;
	CVodeMem cv_mem = data->mem;
	
	
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
	
	if (sundials_report_jac(&data->sd, out, usr) >= 0) ++line;
	
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (CVodeGetCurrentTime(data->mem, &dval) == CV_SUCCESS)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.val.fmt = "D";
	pr.val.ptr = &dval;
	out(usr, &pr);
	++line;
	}
	
	if ((show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)))
	    && (CVodeGetNumSteps(data->mem, &lval) == CV_SUCCESS)) {
	val = lval;
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (CVodeGetLastStep(data->mem, &dval) == CV_SUCCESS)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "D";
	pr.val.ptr = &dval;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	if (CVodeGetNumRhsEvals(data->mem, &lval) == CV_SUCCESS) {
	val = lval;
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	if ((data->sd.linalg & MPT_ENUM(SundialsDls))
	    && (CVodeGetNumLinSolvSetups(data->mem, &lval) == CV_SUCCESS)) {
	val = lval;
	pr.name = "lsetup";
	pr.desc = MPT_tr("linear solver setups");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	if (CVodeGetNumNonlinSolvIters(data->mem, &lval) == CV_SUCCESS) {
	val = lval;
	pr.name = "nliter";
	pr.desc = MPT_tr("nonlinear solver iterations");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	if (CVodeGetNumNonlinSolvConvFails(data->mem, &lval) == CV_SUCCESS && (val = lval)) {
	pr.name = "nlfail";
	pr.desc = MPT_tr("nonlinear solver conv. fail");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	return line;
}

