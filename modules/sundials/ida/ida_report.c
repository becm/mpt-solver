/*!
 * IDA solver status messages
 */

#include <sys/uio.h>

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
	static const char longfmt[] = { MPT_ENUM(TypeLong), 0 };
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
	pr.val.fmt = "d";
	pr.val.ptr = &dval;
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
		static const char fmt[] = {
			MPT_SOLVER_ENUM(SundialsRealtype),
			MPT_value_toVector(MPT_SOLVER_ENUM(SundialsRealtype)),
			0
		};
		struct {
			double t;
			struct iovec s;
		} val;
		size_t pts = ida->ivp.pint + 1;
		
		val.t = ida->t;
		val.s.iov_base = ida->sd.y ? N_VGetArrayPointer(ida->sd.y) : 0;
		val.s.iov_len  = pts * ida->ivp.neqs * sizeof(realtype);
		
		pr.name = 0;
		pr.desc = MPT_tr("IDA solver state");
		pr.val.fmt = fmt;
		pr.val.ptr = &val;
		
		out(usr, &pr);
	}
	
	if ((show & MPT_SOLVER_ENUM(Status))
	    && (IDAGetLastStep(ida->mem, &dval) == IDA_SUCCESS)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.val.fmt = "d";
	pr.val.ptr = &dval;
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



