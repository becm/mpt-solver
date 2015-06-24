/*!
 * output extended information for radau.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "radau.h"

extern int mpt_radau_report(const MPT_SOLVER_STRUCT(radau) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *type; int ml, mu; } miter;
	int neqs;
	
	pr.name = "method";
	pr.desc = MPT_tr("method for jacobian");
	pr.val.fmt = "s";
	pr.val.ptr = &miter;
	
	neqs = data->ivp.neqs * (data->ivp.pint + 1);
	
	if ((miter.ml = data->mljac) >= 0 && miter.ml < neqs) {
		miter.mu   = data->mujac;
		miter.type = data->jac ? "banded(user)" : "banded";
		pr.val.fmt = "sii";
	}
	else {
		miter.type = data->jac ? "full(user)" : "full";
	}
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "D";
	pr.val.ptr = &data->ivp.last;
	out(usr, &pr);
	++line;
	}
	if ((show & MPT_SOLVER_ENUM(Report)) || (show & MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = "integration steps";
	pr.val.fmt = "i";
	pr.val.ptr = &data->count.st.nsteps;
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	pr.val.fmt = "D";
	pr.val.ptr = &data->h;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report)))
		return line;
	
	if (data->count.st.nsteps != data->count.st.naccpt) {
		pr.name = "nacc";
		pr.desc = MPT_tr("accepted steps");
		pr.val.fmt = "i";
		pr.val.ptr = &data->count.st.naccpt;
		out(usr, &pr);
		++line;
	}
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &data->count.st.nfev;
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("Jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &data->count.st.njac;
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &data->count.st.nlud;
	out(usr, &pr);
	++line;
	
	return line;
}

