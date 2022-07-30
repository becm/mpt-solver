/*!
 * print extended information for mebdfi.
 */

#include <stdio.h>
#include <string.h>

#include "mebdfi.h"

#include "module_functions.h"

extern int mpt_mebdfi_report(const MPT_SOLVER_STRUCT(mebdfi) *me, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	int line = 0, *iwk = me->iwork.iov_base;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac;
	pr.name = "jacobian";
	pr.desc = MPT_tr("method for jacobian");
	
	jac = (me->jac && !me->jnum) ? "user" : "numerical";
	
	if (me->jbnd) {
		MPT_STRUCT(property) val[4] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0], (me->jac && !me->jnum) ? "Banded" : "banded");
		val[1].name = "ml";
		val[1].desc = MPT_tr("jacobian lower band size");
		mpt_solver_module_value_int(&val[1], &me->mbnd[0]);
		val[2].name = "mu";
		val[2].desc = MPT_tr("jacobian upper band size");
		mpt_solver_module_value_int(&val[2], &me->mbnd[1]);
		val[3].name = "jac_method";
		val[3].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[3], jac);
		
		mpt_solver_module_report_properties(val, 4, pr.name, pr.desc, out, usr);
	} else {
		MPT_STRUCT(property) val[2] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		
		val[0].name = "jac_type";
		val[0].desc = MPT_tr("jacobian type");
		mpt_solver_module_value_string(&val[0], (me->jac && !me->jnum) ? "Full" : "full");
		val[1].name = "jac_method";
		val[1].desc = MPT_tr("jacobian method");
		mpt_solver_module_value_string(&val[1], jac);
		
		mpt_solver_module_report_properties(val, 2, pr.name, pr.desc, out, usr);
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
	mpt_solver_module_value_double(&pr, &me->t);
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && iwk) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	mpt_solver_module_value_ivec(&pr, 5, &me->iwork);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	mpt_solver_module_value_rvec(&pr, 2, &me->rwork);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Report)) {
	
	pr.name = "nfail";
	pr.desc = MPT_tr("failed steps");
	mpt_solver_module_value_ivec(&pr, 6, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	mpt_solver_module_value_ivec(&pr, 7, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	mpt_solver_module_value_ivec(&pr, 8, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	mpt_solver_module_value_ivec(&pr, 9, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "bsol";
	pr.desc = MPT_tr("backsolves");
	mpt_solver_module_value_ivec(&pr, 10, &me->iwork);
	out(usr, &pr);
	++line;
	
	pr.name = "mform";
	pr.desc = MPT_tr("coeff. matrix form.");
	mpt_solver_module_value_ivec(&pr, 11, &me->iwork);
	out(usr, &pr);
	++line;
	}
	
	return line;
}
