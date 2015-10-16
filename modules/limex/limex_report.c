/*!
 * get LIMEX status information.
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "limex.h"

extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *data, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	struct { const char *jac, *val; int ml, mu; } d;
	int neqs;
	
	pr.name = "jacobian";
	pr.desc = "type of jacobian matrix";
	pr.val.fmt = "ss";
	pr.val.ptr = &d;
	
	neqs = data->ivp.neqs * (data->ivp.pint + 1);
	
	d.val = data->jac ? "(user)" : "(numerical)";
	d.jac = "full";
	
	d.ml = data->iopt[7];
	d.mu = data->iopt[8];
	
	if (d.ml >= 0 && d.ml < neqs) { d.jac = "banded"; pr.val.fmt = "ssii"; }
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "d";
	pr.val.ptr = &data->ivp.last;
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = "integration steps";
	pr.val.fmt = "i";
	pr.val.ptr = &data->iopt[27];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	pr.val.fmt = "d";
	pr.val.ptr = &data->h;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &data->iopt[23];
	out(usr, &pr);
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &data->iopt[28];
	out(usr, &pr);
	
	pr.name = "jfeval";
	pr.desc = "jacobian f eval.";
	pr.val.fmt = "i";
	pr.val.ptr = &data->iopt[24];
	out(usr, &pr);
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &data->iopt[25];
	out(usr, &pr);
	
	pr.name = "lubsub";
	pr.desc = MPT_tr("LU back-subst.");
	pr.val.fmt = "i";
	pr.val.ptr = &data->iopt[26];
	out(usr, &pr);
	
	return line + 6;
}
