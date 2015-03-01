/*!
 * print extended information for mebdfi.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "mebdfi.h"

extern int mpt_mebdfi_report(const MPT_SOLVER_STRUCT(mebdfi) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0, *iwk = data->iwork.iov_base;
	double *rwk = data->rwork.iov_base;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	pr.name = "method";
	pr.desc = MPT_tr("method for jacobian");
	if (data->jbnd) {
		struct { const char *t; int mu, ml; } val;
		val.t = (data->jac && !data->jnum) ? "banded(user)" : "banded";
		val.t = MPT_tr(val.t);
		val.ml = data->mbnd[0]; val.mu = data->mbnd[0];
		pr.fmt  = "sii";
		pr.data = &val;
		out(usr, &pr);
	} else {
		pr.fmt  = 0;
		pr.data = (data->jac && !data->jnum) ? "full(user)" : "full";
		pr.data = MPT_tr(pr.data);
		out(usr, &pr);
	}
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = MPT_tr("value of independent variable");
	pr.fmt  = "G";
	pr.data = &data->ivp.last;
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report)) && iwk) {
	pr.name = "n";
	pr.desc = MPT_tr("integration steps");
	pr.fmt  = "i";
	pr.data = &iwk[4];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status) && rwk) {
	pr.name = "h";
	pr.desc = MPT_tr("current step size");
	pr.fmt  = "G";
	pr.data = &rwk[1];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Report) && iwk) {
	
	if (iwk[5]) {
	pr.name = "nfail";
	pr.desc = MPT_tr("failed steps");
	pr.fmt  = "i";
	pr.data = &iwk[5];
	out(usr, &pr);
	++line;
	}
	
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	pr.fmt  = "i";
	pr.data = &iwk[6];
	out(usr, &pr);
	++line;
	
	pr.name = "jeval";
	pr.desc = MPT_tr("Jacobian evaluations");
	pr.fmt  = "i";
	pr.data = &iwk[7];
	out(usr, &pr);
	++line;
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.fmt  = "i";
	pr.data = &iwk[8];
	out(usr, &pr);
	++line;
	
	pr.name = "bsol";
	pr.desc = MPT_tr("backsolves");
	pr.fmt  = "i";
	pr.data = &iwk[9];
	out(usr, &pr);
	++line;
	
	pr.name = "mform";
	pr.desc = MPT_tr("coeff. matrix form.");
	pr.fmt  = "i";
	pr.data = &iwk[10];
	out(usr, &pr);
	++line;
	}
	
	return line;
}
