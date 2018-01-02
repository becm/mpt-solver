/*!
 * IDA solver status messages
 */

#include <sys/uio.h>

#include <ida/ida_spils.h>
#include <ida/ida_direct.h>
#include <ida/ida.h>

#include "sundials.h"

#include "module_functions.h"

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
	static const uint8_t longfmt[] = "l\0";
	MPT_STRUCT(property) pr;
	long int lval;
	double dval;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	if (sundials_report_jac(&ida->sd, out, usr) > 0) ++line;
	}
	
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (IDAGetCurrentTime(ida->mem, &dval) == IDA_SUCCESS)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	mpt_solver_module_value_double(&pr.val, &dval);
	out(usr, &pr);
	++line;
	}
	if ((show & (MPT_SOLVER_ENUM(Report) | MPT_SOLVER_ENUM(Status)))
	    && (IDAGetNumSteps(ida->mem, &lval) == IDA_SUCCESS)) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	realtype *val = ida->sd.y ? N_VGetArrayPointer(ida->sd.y) : 0;
	MPT_SOLVER_MODULE_FCN(ivp_values)(&ida->ivp, ida->t, val, MPT_tr("IDA solver state"), out, usr);
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (IDAGetLastStep(ida->mem, &dval) == IDA_SUCCESS)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	mpt_solver_module_value_double(&pr.val, &dval);
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	if (IDAGetNumResEvals(ida->mem, &lval) == IDA_SUCCESS) {
	pr.name = "reval";
	pr.desc = MPT_tr("residual evaluations");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	
	if (ida->sd.linalg & MPT_SOLVER_ENUM(SundialsSpils)) {
		if (IDASpilsGetNumLinIters(ida->mem, &lval) == IDA_SUCCESS) {
		pr.name = "liter";
		pr.desc = MPT_tr("linear iterations");
		pr.val.fmt = longfmt;
		pr.val.ptr = &lval;
		out(usr, &pr);
		++line;
		}
	} else if (ida->sd.linalg & MPT_SOLVER_ENUM(SundialsDls)) {
		if (IDADlsGetNumJacEvals(ida->mem, &lval) == IDA_SUCCESS) {
		pr.name = "jeval";
		pr.desc = MPT_tr("jacobian evaluations");
		pr.val.fmt = longfmt;
		pr.val.ptr = &lval;
		out(usr, &pr);
		++line;
		}
	}
	if (IDAGetNumNonlinSolvIters(ida->mem, &lval) == IDA_SUCCESS) {
	pr.name = "niter";
	pr.desc = MPT_tr("nonlinear iterations");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	if ((IDAGetNumErrTestFails(ida->mem, &lval) == IDA_SUCCESS)
	    && lval) {
	pr.name = "etfail";
	pr.desc = MPT_tr("error test failures");
	pr.val.fmt = longfmt;
	pr.val.ptr = &lval;
	out(usr, &pr);
	++line;
	}
	return line;
}



