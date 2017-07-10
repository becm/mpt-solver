/*!
 * get BACOL status information
 */

#include <string.h>

#include "bacol.h"

extern int mpt_bacol_report(const MPT_SOLVER_STRUCT(bacol) *bac, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int lines = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *backend;
	int32_t kcol = bac->kcol;
	
	switch (bac->_backend) {
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
	
	if (show & MPT_SOLVER_ENUM(Values)) {
		const char fmt[] = { 'd', 0 };
		double t = bac->t;

		pr.name = 0;
		pr.desc = MPT_tr("BACOL solver state");
		pr.val.fmt = fmt;
		pr.val.ptr = &t;
		out(usr, &pr);
	}
	return lines;
}
