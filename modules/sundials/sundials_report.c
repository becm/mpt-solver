/*!
 * report for SUNDIALS jacobian operations
 */

#include <stdio.h>
#include <sundials/sundials_direct.h>

#include "sundials.h"

extern int mpt_sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *sd, MPT_TYPE(PropertyHandler) out, void *usr)
{
	static const uint8_t fmt_full[] = "s";
	static const uint8_t fmt_band[] = "sii";
	MPT_STRUCT(property) pr;
	struct {
		const char *type;
		int mu, ml;
	} val;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = fmt_full;
	pr.val.ptr = &val;
	
	switch (sd->jacobian) {
	    case 0:
		val.type = "diag";
		break;
	    case MPT_SOLVER_SUNDIALS(Direct):
	    case MPT_SOLVER_SUNDIALS(Lapack):
		val.type = sd->jacobian == SUNDIALS_BAND ? "banded(user)" : "full(user)";
		break;
	    case MPT_SOLVER_SUNDIALS(Direct) | MPT_SOLVER_SUNDIALS(Numeric):
	    case MPT_SOLVER_SUNDIALS(Lapack) | MPT_SOLVER_SUNDIALS(Numeric):
		val.type = sd->jacobian == SUNDIALS_BAND ? "banded" : "full";
	    default:
		return 0;
	}
	if (sd->jacobian == SUNDIALS_BAND) {
		pr.val.fmt = fmt_band;
		val.mu = sd->mu;
		val.ml = sd->ml;
	}
	return out(usr, &pr);
}

