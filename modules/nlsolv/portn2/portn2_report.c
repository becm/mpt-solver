/*!
 * get status information for PORT N2 instance
 */

#include "portn2.h"

extern int mpt_portn2_report(const MPT_SOLVER_STRUCT(portn2) *n2, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int state, line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *jac = "user (partial)";
	
	if (n2->nd < 0)   jac = "numeric";
	else if (!n2->nd) jac = "user (full)";
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = 0;
	pr.val.ptr = jac;
	out(usr, &pr);
	++line;
	}
	state = ((int *) n2->iv.iov_base)[0];
	if ((show & MPT_SOLVER_ENUM(Values)) && state >= 0) {
		static const uint8_t fmt[] = { MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
		struct {
			struct iovec x, f;
		} d;
		
		d.x.iov_base = n2->pv.iov_base;
		d.x.iov_len  = n2->nls.nval * sizeof(double);
		
		pr.name = 0;
		pr.desc = MPT_tr("parameters and residual data");
		pr.val.fmt = fmt+1;
		pr.val.ptr = &d;
		
		if (state && (d.f.iov_base = (void *) mpt_portn2_residuals(n2))) {
			d.f.iov_len = n2->nls.nres * sizeof(double);
			pr.val.fmt  = fmt;
		}
		out(usr, &pr);
	}
	
	return 0;
}
