/*!
 * get BACOL status information
 */

#include "bacol.h"

extern int mpt_bacol_report(const MPT_SOLVER_STRUCT(bacol) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	const char *backend;
	int kcol = data->kcol;
	int lines = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	
	switch (data->backend) {
	    case 'r': case 'R': backend = "radau"; break;
	    case 'd': case 'D': backend = "dassl"; break;
	    default: backend = "(unknown)";
	}
	
	pr.name = "backend";
	pr.desc = "used single step backend";
	pr.fmt  = 0;
	pr.data = backend;
	out(usr, &pr);
	++lines;
	
	pr.name = "kcol";
	pr.desc = "collocations";
	pr.fmt  = "i";
	pr.data = &kcol;
	out(usr, &pr);
	++lines;
	}
	
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.fmt  = "d";
	pr.data = &data->ivp.last;
	out(usr, &pr);
	++lines;
	
	pr.name = "nint";
	pr.desc = "intervals";
	pr.fmt  = "i";
	pr.data = &data->nint;
	out(usr, &pr);
	++lines;
	}
	
	return lines;
}
