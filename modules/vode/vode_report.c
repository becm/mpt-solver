/*!
 * prepare/finalize VODE solver,
 * get status information.
 */

#include "vode.h"

#include "module_functions.h"

extern int mpt_vode_report(const MPT_SOLVER_STRUCT(vode) *vd, int show, MPT_TYPE(property_handler) out, void *usr)
{
	MPT_STRUCT(property) pr = MPT_PROPERTY_INIT;
	MPT_STRUCT(property) more[4] = {
		MPT_PROPERTY_INIT,
		MPT_PROPERTY_INIT,
		MPT_PROPERTY_INIT,
		MPT_PROPERTY_INIT
	};
	size_t li = vd->iwork.iov_len / sizeof(int);
	size_t lr = vd->rwork.iov_len / sizeof(double);
	int *iwk = vd->iwork.iov_base;
	double *rwk = vd->rwork.iov_base;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *val;
	
	pr.name = "method";
	pr.desc = MPT_tr("method for solver step");
	
	val = (vd->meth == 2) ? "BDF" : "Adams";
	
	if (!vd->miter || vd->jsv < 0) {
		mpt_solver_module_value_string(&pr.val, val);
		out(usr, &pr);
	}
	else {
		more[0].name = "sol_method";
		more[0].desc = MPT_tr("solver method");
		mpt_solver_module_value_string(&more[0].val, val);
		
		more[1].name = "sol_save";
		more[1].desc = MPT_tr("reuse data");
		val = "saved";
		mpt_solver_module_value_string(&more[1].val, val);
		
		mpt_solver_module_report_properties(more, 2, pr.name, pr.desc, out, usr);
	}
	++line;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("type of jacobian");
	
	if (li > 1) {
		int32_t ml = iwk[0];
		int32_t mu = iwk[1];
		
		more[1].name = "ml";
		more[1].desc = MPT_tr("jacobian lower band size");
		MPT_value_set_data(&more[1].val, 'i', &ml);
		
		more[2].name = "mu";
		more[2].desc = MPT_tr("jacobian upper band size");
		MPT_value_set_data(&more[2].val, 'i', &mu);
	}
	
	switch (vd->miter) {
		case 1:
			val = "Full";
			if (vd->jac) {
				mpt_solver_module_value_string(&pr.val, val);
				out(usr, &pr);
				break;
			}
			/* fall through */
		case 2:
			val = "full";
			if (vd->jac) {
				more[0].name = "jac_method";
				more[0].desc = MPT_tr("jacobian method");
				mpt_solver_module_value_string(&more[0].val, val);
				
				more[1].name = "jac_type";
				more[1].desc = MPT_tr("jacobian type");
				val = "numerical";
				mpt_solver_module_value_string(&more[1].val, val);
				
				mpt_solver_module_report_properties(more, 2, pr.name, pr.desc, out, usr);
			}
			else {
				mpt_solver_module_value_string(&pr.val, val);
				out(usr, &pr);
			}
			break;
		case 3:
			val = "diagonal";
			mpt_solver_module_value_string(&pr.val, val);
			out(usr, &pr);
			break;
		case 4:
			val = "Banded";
			if (vd->jac) {
				more[0].name = "jac_method";
				more[0].desc = MPT_tr("jacobian method");
				mpt_solver_module_value_string(&more[0].val, val);
				
				mpt_solver_module_report_properties(more, li > 1 ? 3 : 1, pr.name, pr.desc, out, usr);
				break;
			}
			/* fall through */
		case 5:
			val = "banded";
			more[0].name = "jac_method";
			more[0].desc = MPT_tr("jacobian method");
			mpt_solver_module_value_string(&more[0].val, val);
			
			val = "numerical";
			if (li > 1) {
				if (vd->jac) {
					more[3].name = "jac_type";
					more[3].desc = MPT_tr("jacobian type");
					val = "numerical";
					mpt_solver_module_value_string(&more[3].val, val);
					
				}
				mpt_solver_module_report_properties(more, vd->jac ? 4 : 3, pr.name, pr.desc, out, usr);
			}
			else {
				if (vd->jac) {
					more[1].name = "jac_type";
					more[1].desc = MPT_tr("jacobian type");
					val = "numerical";
					mpt_solver_module_value_string(&more[1].val, val);
				}
				mpt_solver_module_report_properties(more, vd->jac ? 2 : 1, pr.name, pr.desc, out, usr);
			}
			break;
		default:
			val = "none";
			mpt_solver_module_value_string(&pr.val, val);
			out(usr, &pr);
	}
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&vd->ivp, vd->t, vd->y, MPT_tr("dVode solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	if (lr > 12) {
		mpt_solver_module_value_double(&pr.val, &rwk[12]);
	} else {
		mpt_solver_module_value_double(&pr.val, &vd->t);
	}
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && li > 10) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	mpt_solver_module_value_int(&pr.val, &iwk[10]);
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && lr > 10) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	mpt_solver_module_value_double(&pr.val, &rwk[10]);
	out(usr, &pr);
	++line;
	}
	
	if ((show & MPT_SOLVER_ENUM(Report)) && li > 11) {
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	mpt_solver_module_value_int(&pr.val, &iwk[11]);
	out(usr, &pr);
	++line;
	
	if (li > 12 && iwk[12]) {
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	mpt_solver_module_value_int(&pr.val, &iwk[12]);
	out(usr, &pr);
	++line;
	}
	
	if (li > 18 && iwk[18]) {
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	mpt_solver_module_value_int(&pr.val, &iwk[18]);
	out(usr, &pr);
	++line;
	}
	
	if (li > 19 && iwk[19]) {
	pr.name = "niter";
	pr.desc = MPT_tr("nonlinear (Newton) iterations");
	mpt_solver_module_value_int(&pr.val, &iwk[19]);
	out(usr, &pr);
	++line;
	}
	}
	return line;
}
