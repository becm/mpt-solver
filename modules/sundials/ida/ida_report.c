/*!
 * IDA solver status messages
 */

#include <errno.h>

#include <ida/ida_spils.h>
#include <ida/ida_direct.h>
#include <ida/ida.h>

#include "sundials.h"

/*!
 * \ingroup mptSundialsIda
 * \brief IDA solver report
 * 
 * Process IDA report information.
 * Report properties are emitted to passed handler.
 * 
 * \param ida  IDA solver data
 * \param show types of report entries
 * \param out  handler for report properties
 * \param usr  data context for handler
 * 
 * \return number of reported properties
 */
extern int sundials_ida_report(const MPT_SOLVER_STRUCT(ida) *ida, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	double dval;
	long lval;
	int64_t val;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	if (sundials_report_jac(&ida->sd, out, usr) > 0) ++line;
	}
	
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (IDAGetCurrentTime(ida->mem, &dval) == IDA_SUCCESS)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.val.fmt = "G";
	pr.val.ptr = &dval;
	out(usr, &pr);
	++line;
	}
	if ((show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status)))
	    && (IDAGetNumSteps(ida->mem, &lval) == IDA_SUCCESS)) {
	val = lval;
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (IDAGetLastStep(ida->mem, &dval) == IDA_SUCCESS)) {
	val = lval;
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "G";
	pr.val.ptr = &dval;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	if (IDAGetNumResEvals(ida->mem, &lval) == IDA_SUCCESS) {
	val = lval;
	pr.name = "reval";
	pr.desc = MPT_tr("residual evaluations");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	
	if (ida->sd.linalg & MPT_ENUM(SundialsSpils)) {
		if (IDASpilsGetNumLinIters(ida->mem, &lval) == IDA_SUCCESS) {
		val = lval;
		pr.name = "liter";
		pr.desc = MPT_tr("linear iterations");
		pr.val.fmt = "l";
		pr.val.ptr = &val;
		out(usr, &pr);
		++line;
		}
	} else if (ida->sd.linalg & MPT_ENUM(SundialsDls)) {
		if (IDADlsGetNumJacEvals(ida->mem, &lval) == IDA_SUCCESS) {
		val = lval;
		pr.name = "jeval";
		pr.desc = MPT_tr("jacobian evaluations");
		pr.val.fmt = "l";
		pr.val.ptr = &val;
		out(usr, &pr);
		++line;
		}
	}
	if (IDAGetNumNonlinSolvIters(ida->mem, &lval) == IDA_SUCCESS) {
	val = lval;
	pr.name = "niter";
	pr.desc = MPT_tr("nonlinear iterations");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	if ((IDAGetNumErrTestFails(ida->mem, &lval) == IDA_SUCCESS)
	    && (val = lval)) {
	pr.name = "etfail";
	pr.desc = MPT_tr("error test failures");
	pr.val.fmt = "l";
	pr.val.ptr = &val;
	out(usr, &pr);
	++line;
	}
	return line;
}



