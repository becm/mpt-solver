/*!
 * report for SUNDIALS jacobian operations
 */

#include <sundials/sundials_matrix.h>

#include <sunmatrix/sunmatrix_band.h>

#include "sundials.h"

extern int mpt_sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *sd, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	SUNMatrix A;
	SUNMatrix_ID type;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = 0;
	
	if (sd->linsol < MPT_SOLVER_SUNDIALS(Direct)) {
		pr.val.ptr = "none";
		return out(usr, &pr);
	}
	if (!(A = sd->A)) {
		pr.val.ptr = "diagonal";
		return out(usr, &pr);
	}
	type = SUNMatGetID(A);
	
	if (type == SUNMATRIX_DENSE) {
		static const uint8_t fmt[] = "ss";
		const char *val[2];
		
		val[0] = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "full" : "Full";
		val[1] = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "numerical": "user";
		
		pr.val.fmt = fmt;
		pr.val.ptr = val;
		return out(usr, &pr);
	}
	if (type == SUNMATRIX_BAND) {
		static const uint8_t fmt[] = "siis";
		struct { const char *fmt; int32_t mu, ml; const char *jac; } val;
		
		val.fmt = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "banded" : "Banded";
		val.mu  = SUNBandMatrix_UpperBandwidth(A);
		val.ml  = SUNBandMatrix_LowerBandwidth(A);
		val.jac = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "numerical": "user";
		
		pr.val.fmt = fmt;
		pr.val.ptr = &val;
		return out(usr, &pr);
	}
	return 0;
}

