/*!
 * get BACOL status information
 */

#include <string.h>

#include "bacol.h"

extern int mpt_bacol_report(MPT_SOLVER_STRUCT(bacol) *bac, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int len, lines = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *backend;
	int32_t kcol = bac->kcol;
	
	switch (bac->backend) {
	    case 'r': case 'R': backend = "radau"; break;
	    case 'd': case 'D': backend = "dassl"; break;
	    default: backend = "(unknown)";
	}
	
	pr.name = "backend";
	pr.desc = "used single step backend";
	pr.val.fmt = 0;
	pr.val.ptr = backend;
	out(usr, &pr);
	++lines;
	
	pr.name = "kcol";
	pr.desc = "collocations";
	pr.val.fmt = "i";
	pr.val.ptr = &kcol;
	out(usr, &pr);
	++lines;
	}
	
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "d";
	pr.val.ptr = &bac->t;
	out(usr, &pr);
	++lines;
	
	pr.name = "nint";
	pr.desc = "intervals";
	pr.val.fmt = "i";
	pr.val.ptr = &bac->nint;
	out(usr, &pr);
	++lines;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)
	    && (len = mpt_bacol_values(bac, bac->out.x, 0, bac->out.y)) > 0) {
		static const char fmt[] = { 'd', MPT_value_toVector('d'), MPT_value_toVector('d'), 0 };
		struct {
			double t;
			struct iovec x, y;
		} s;
		s.t = bac->t;
		s.x.iov_base = bac->out.x;
		s.x.iov_len  = len * sizeof(*bac->out.x);
		s.y.iov_base = bac->out.y;
		s.y.iov_len  = bac->ivp.neqs * s.x.iov_len;
		
		pr.name = 0;
		pr.desc = MPT_tr("solver state");
		pr.val.fmt = fmt;
		pr.val.ptr = &s;
		out(usr, &pr);
	}
	return lines;
}
