/*!
 * report for SUNDIALS jacobian operations
 */

#include <sundials/sundials_matrix.h>

#include <sunmatrix/sunmatrix_band.h>

#include "sundials.h"

extern int mpt_sundials_report_jac(const MPT_SOLVER_STRUCT(sundials) *sd, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	SUNMatrix A;
	SUNMatrix_ID type;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.type = 's';
	pr.val.ptr = pr.val._buf;
	
	if (sd->linsol < MPT_SOLVER_SUNDIALS(Direct)) {
		*((const char **) pr.val._buf) = "diagonal";
		return out(usr, &pr);
	}
	if (!(A = sd->A)) {
		*((const char **) pr.val._buf) = "diagonal";
		return out(usr, &pr);
	}
	type = SUNMatGetID(A);
	
	if (type == SUNMATRIX_DENSE) {
		MPT_STRUCT(property) prop[2] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		const char *ptr;
		
		prop[0].name = "jac_type";
		prop[0].desc = MPT_tr("jacobian type");
		prop[0].val.type = 's';
		ptr = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "full" : "Full";
		mpt_solver_module_value_string(&prop[0].val, ptr);
		
		prop[1].name = "jac_method";
		prop[1].desc = MPT_tr("jacobian method");
		prop[1].val.type = 's';
		ptr = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "numerical": "user";
		mpt_solver_module_value_string(&prop[1].val, ptr);
		
		mpt_solver_module_report_properties(prop, 2, pr.name, pr.desc, out, usr);
	}
	if (type == SUNMATRIX_BAND) {
		MPT_STRUCT(property) prop[4] = {
			MPT_PROPERTY_INIT,
			MPT_PROPERTY_INIT,
			MPT_PROPERTY_INIT,
			MPT_PROPERTY_INIT
		};
		const char *ptr;
		int32_t val;
		
		prop[0].name = "jac_type";
		prop[0].desc = MPT_tr("jacobian type");
		ptr = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "full" : "Full";
		mpt_solver_module_value_string(&prop[0].val, ptr);
		
		prop[1].name = "ml";
		prop[1].desc = MPT_tr("jacobian lower band size");
		val = SUNBandMatrix_LowerBandwidth(A);
		MPT_value_set_data(&prop[1].val, 'i', &val);
		
		prop[2].name = "mu";
		prop[2].desc = MPT_tr("jacobian upper band size");
		val = SUNBandMatrix_LowerBandwidth(A);
		MPT_value_set_data(&prop[2].val, 'i', &val);
		
		prop[3].name = "jac_method";
		prop[3].desc = MPT_tr("jacobian method");
		ptr = sd->linsol & MPT_SOLVER_SUNDIALS(Numeric) ? "numerical": "user";
		mpt_solver_module_value_string(&prop[3].val, ptr);
		
		mpt_solver_module_report_properties(prop, 4, pr.name, pr.desc, out, usr);
	}
	return 0;
}

