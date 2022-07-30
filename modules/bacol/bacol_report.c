/*!
 * get BACOL status information
 */

#include <string.h>

#include "bacol.h"

extern int mpt_bacol_report(const MPT_SOLVER_STRUCT(bacol) *bac, const MPT_SOLVER_STRUCT(bacol_out) *od, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
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
	mpt_solver_module_value_string(&pr, backend);
	out(usr, &pr);
	++lines;
	
	pr.name = "kcol";
	pr.desc = "collocations";
	mpt_solver_module_value_int(&pr, &kcol);
	out(usr, &pr);
	++lines;
	}
	
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	mpt_solver_module_value_double(&pr, &bac->t);
	out(usr, &pr);
	++lines;
	
	pr.name = "nint";
	pr.desc = "intervals";
	mpt_solver_module_value_int(&pr, &bac->nint);
	out(usr, &pr);
	++lines;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	pr.name = 0;
	pr.desc = MPT_tr("BACOL solver state");
	if (!od) {
		mpt_solver_module_value_double(&pr, &bac->t);
		out(usr, &pr);
	}
	else {
		MPT_STRUCT(property) val[3] = { MPT_PROPERTY_INIT, MPT_PROPERTY_INIT, MPT_PROPERTY_INIT };
		int ret;
		
		val[0].name = "t";
		val[0].desc = MPT_tr("current time");
		mpt_solver_module_value_double(&val[0], &bac->t);
		
		val[1].name = "grid";
		val[1].desc = MPT_tr("user-defined output grid data");
		
		val[2].name = "y";
		val[2].desc = MPT_tr("data interpolation for output grid");
		
		if ((ret = mpt_bacol_output_grid(od, (void *) val[1]._buf) < 0)
		 || (ret = mpt_bacol_output_values(od, (void *) val[2]._buf) < 0)) {
			out(usr, &val[0]);
		}
		else {
			MPT_value_set(&val[1].val, MPT_type_toVector('d'), val[1]._buf);
			MPT_value_set(&val[2].val, MPT_type_toVector('d'), val[2]._buf);
			mpt_solver_module_report_properties(val, 3, pr.name, pr.desc, out, usr);
		}
	}
	}
	return lines;
}
