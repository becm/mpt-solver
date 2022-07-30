/*!
 * MPT solver module helper function
 *   dispatch IVP state data via property handler
 */

#include "solver_modfcn.h"

extern int MPT_SOLVER_MODULE_FCN(ivp_values)(const MPT_IVP_STRUCT(parameters) *ivp, double t, const MPT_SOLVER_MODULE_DATA_TYPE *y, const char *desc, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr[3] = {
		MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT
	};
	struct iovec *vec;
	size_t len = ivp->pint;
	
	pr[0].name = "t";
	pr[0].desc = "current time";
	MPT_property_set_data(&pr[0], 'd', &t);
	
	vec = (void *) pr[1]._buf;
	vec->iov_base = (void *) y;
	vec->iov_len  = ivp->neqs * sizeof(*y);
	pr[1].name = "y";
	pr[1].desc = "current state";
	MPT_value_set(&pr[1].val, MPT_type_toVector(MPT_SOLVER_MODULE_DATA_ID), vec);
	
	if (!len++) {
		return mpt_solver_module_report_properties(pr, 2, 0, desc, out, usr);
	}
	vec->iov_len = len * ivp->neqs * sizeof(*y);
	
	vec = (void *) pr[2]._buf;
	vec->iov_base = ivp->grid;
	vec->iov_len  = len * sizeof(*ivp->grid);
	pr[2].name = "grid";
	pr[2].desc = "grid data";
	MPT_value_set(&pr[2].val, MPT_type_toVector('d'), vec);
	
	return mpt_solver_module_report_properties(pr, 3, 0, desc, out, usr);
}

