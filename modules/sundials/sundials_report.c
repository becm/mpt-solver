/*!
 * report for SUNDIALS jacobian operations
 */

#include <stdio.h>
#include <sundials/sundials_direct.h>

#include "sundials.h"

extern int mpt_sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *sd, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	static const uint8_t fmt[] = "s";
	const char *jac;
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = fmt;
	pr.val.ptr = &jac;
	
	if (sd->stype < MPT_SOLVER_SUNDIALS(Direct)) {
		jac = "none";
		return out(usr, &pr);
	}
	if (!sd->jacobian) {
		jac = "diag";
		return out(usr, &pr);
	}
	if (sd->jacobian == SUNDIALS_DENSE) {
		static const uint8_t fmt[] = "ss";
		const char *val[2];
		
		val[0] = sd->stype & MPT_SOLVER_SUNDIALS(Numeric) ? "full" : "Full";
		val[1] = sd->stype & MPT_SOLVER_SUNDIALS(Numeric) ? "numerical": "user";
		
		pr.val.fmt = fmt;
		pr.val.ptr = val;
		return out(usr, &pr);
	}
	if (sd->jacobian == SUNDIALS_BAND) {
		static const uint8_t fmt[] = "siis";
		struct { const char *fmt; int32_t mu, ml; const char *jac; } val;
		
		val.fmt = sd->stype & MPT_SOLVER_SUNDIALS(Numeric) ? "banded" : "Banded";
		val.mu  = sd->mu;
		val.ml  = sd->ml;
		val.jac = sd->stype & MPT_SOLVER_SUNDIALS(Numeric) ? "numerical": "user";
		
		pr.val.fmt = fmt;
		pr.val.ptr = &val;
		return out(usr, &pr);
	}
	return 0;
}

