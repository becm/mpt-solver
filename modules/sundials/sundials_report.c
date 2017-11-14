/*!
 * report for CVode solver
 */

#include "sundials.h"

extern int sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *sd, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	struct {
		const char *type;
		int mu, ml;
	} val;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = "s";
	pr.val.ptr = &val;
	
	switch (sd->jacobian) {
		case MPT_SOLVER_ENUM(SundialsJacDiag):
		case MPT_SOLVER_ENUM(SundialsJacDiag) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			val.type = "diag"; break;
		case MPT_SOLVER_ENUM(SundialsJacDense):
			val.type = "full(user)"; break;
		case MPT_SOLVER_ENUM(SundialsJacDense) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			val.type = "full"; break;
		case MPT_SOLVER_ENUM(SundialsJacBand):
			val.type = "banded(user)"; pr.val.fmt = "sii";
			val.mu = sd->mu; val.ml = sd->ml;
			break;
		case MPT_SOLVER_ENUM(SundialsJacBand) | MPT_SOLVER_ENUM(SundialsJacNumeric):
			val.type = "banded"; pr.val.fmt = "sii";
			val.mu = sd->mu; val.ml = sd->ml;
			break;
		default:
			return 0;
	}
	return out(usr, &pr);
}
