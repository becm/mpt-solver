/*!
 * get status information for PORT N2 instance
 */

#include <errno.h>

#include "portn2.h"

extern int mpt_portn2_report(const MPT_SOLVER_STRUCT(portn2) *n2, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0;
	
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
	
	return 0;
}
