/*!
 * print extended information for mebdfi.
 */

#include <stdio.h>
#include <string.h>

#include "mebdfi.h"

#include "module_functions.h"

extern int mpt_mebdfi_report(const MPT_SOLVER_STRUCT(mebdfi) *me, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0, *iwk = me->iwork.iov_base;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac;
	pr.name = "jacobian";
	pr.desc = MPT_tr("method for jacobian");
	
	jac = (me->jac && !me->jnum) ? "user" : "numerical";
	
	if (me->jbnd) {
		static const uint8_t fmt[] = "siis";
		struct { const char *fmt; int32_t mu, ml; const char *jac; } val;
		
		val.fmt = (me->jac && !me->jnum) ? "Banded" : "banded";
		val.ml  = me->mbnd[0];
		val.mu  = me->mbnd[1];
		val.jac = jac;
		
		pr.val.fmt = fmt;
		pr.val.ptr = &val;
		out(usr, &pr);
	} else {
		static const uint8_t fmt[] = "ss";
		const char *val[2];
		
		val[0] = (me->jac && !me->jnum) ? "Full" : "full";
		val[1] = jac;
		
		pr.val.fmt = fmt;
		pr.val.ptr = val;
		out(usr, &pr);
	}
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&me->ivp, me->t, me->y, MPT_tr("MEBDFI solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)
	    && me->rwork.iov_base) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	mpt_solver_module_value_double(&pr.val, &me->t);
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && iwk) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	mpt_solver_module_value_ivec(&pr.val, 5, &me->iwork);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	mpt_solver_module_value_rvec(&pr.val, 2, &me->rwork);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Report)) {
	
	pr.name = "nfail";
	pr.desc = MPT_tr("failed steps");
	mpt_solver_module_value_ivec(&pr.val, 6, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	mpt_solver_module_value_ivec(&pr.val, 7, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	mpt_solver_module_value_ivec(&pr.val, 8, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	mpt_solver_module_value_ivec(&pr.val, 9, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "bsol";
	pr.desc = MPT_tr("backsolves");
	mpt_solver_module_value_ivec(&pr.val, 10, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "mform";
	pr.desc = MPT_tr("coeff. matrix form.");
	mpt_solver_module_value_ivec(&pr.val, 11, &me->iwork);
	out(usr, &pr);
	++line;
	}
	
	return line;
}
