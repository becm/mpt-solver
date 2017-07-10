/*!
 * output extended information for radau.
 */

#include "radau.h"

#include "daesolv_modfcn.h"

extern int mpt_radau_report(const MPT_SOLVER_STRUCT(radau) *rd, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int32_t val;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *type; int32_t ml, mu; } miter;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("method for jacobian");
	pr.val.fmt = "s";
	pr.val.ptr = &miter;
	
	val = rd->ivp.neqs * (rd->ivp.pint + 1);
	
	if ((miter.ml = rd->mljac) >= 0 && miter.ml < val) {
		miter.mu   = rd->mujac;
		miter.type = rd->jac ? "banded(user)" : "banded";
		pr.val.fmt = "sii";
	}
	else {
		miter.type = rd->jac ? "full(user)" : "full";
	}
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	MPT_SOLVER_MODULE_FCN(ivp_values)(&rd->ivp, rd->t, rd->y, MPT_tr("RADAU solver state"), out, usr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "d";
	pr.val.ptr = &rd->t;
	out(usr, &pr);
	++line;
	}
	if ((show & MPT_SOLVER_ENUM(Report)) || (show & MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = "integration steps";
	pr.val.fmt = "i";
	pr.val.ptr = &val; val = rd->count.st.nsteps;
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	pr.val.fmt = "d";
	pr.val.ptr = &rd->h;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) {
		return line;
	}
	
	if (rd->count.st.nsteps != rd->count.st.naccpt) {
		pr.name = "nacc";
		pr.desc = MPT_tr("accepted steps");
		pr.val.fmt = "i";
		pr.val.ptr = &val; val = rd->count.st.naccpt;
		out(usr, &pr);
		++line;
	}
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &val; val = rd->count.st.nfev;
	if (val < 0) val = 0;
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &val; val = rd->count.st.njac;
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &val; val = rd->count.st.nlud;
	out(usr, &pr);
	++line;
	
	return line;
}

