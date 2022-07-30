/*!
 * get status information for PORT N2 instance
 */

#include "types.h"

#include "portn2.h"

extern int mpt_portn2_report(const MPT_SOLVER_STRUCT(portn2) *n2, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr[] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
	int state, line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac = "user (partial)";
	
	if (n2->nd < 0)   jac = "numeric";
	else if (!n2->nd) jac = "user (full)";
	
	pr[0].name = "jacobian";
	pr[0].desc = "type of jacobian matrix";
	mpt_solver_module_value_string(&pr[0], jac);
	out(usr, &pr[0]);
	++line;
	}
	state = ((int *) n2->iv.iov_base)[0];
	if ((show & MPT_SOLVER_ENUM(Values)) && state >= 0) {
		struct iovec *vec;
		const double *res;
		
		pr[0].name = 0;
		pr[0].desc = MPT_tr("current parameter values");
		
		vec = (void *) pr[0]._buf;
		vec->iov_base = n2->pv.iov_base;
		vec->iov_len  = n2->nls.nval * sizeof(double);
		MPT_value_set(&pr[0].val, MPT_type_toVector('d'), vec);
		
		if (state && (res = mpt_portn2_residuals(n2))) {
			const char *desc = MPT_tr("current parameters and residuals");
			
			pr[0].name = "parameters";
			
			pr[1].name = "residuals";
			pr[1].desc = MPT_tr("residuals for current parameters");
			
			vec = (void *) pr[1]._buf;
			vec->iov_base = (void *) res;
			vec->iov_len  = n2->nls.nres * sizeof(double);
			MPT_value_set(&pr[1].val, MPT_type_toVector('d'), vec);
			
			mpt_solver_module_report_properties(pr, 2, 0, desc, out, usr); 
		}
		else {
			out(usr, &pr[0]);
		}
	}
	
	return 0;
}
