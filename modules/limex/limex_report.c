/*!
 * get LIMEX status information.
 */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "limex.h"

extern int mpt_limex_report(const MPT_SOLVER_STRUCT(limex) *lx, int show, MPT_TYPE(PropertyHandler) out, void *usr)
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
	
	neqs = lx->ivp.neqs * (lx->ivp.pint + 1);
	
	d.val = lx->jac ? "(user)" : "(numerical)";
	d.jac = "full";
	
	d.ml = lx->iopt[7];
	d.mu = lx->iopt[8];
	
	if (d.ml >= 0 && d.ml < neqs) { d.jac = "banded"; pr.val.fmt = "ssii"; }
	
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Values)) {
	static const char fmt[] = { 'd', MPT_value_toVector('d'), 0 };
	struct {
		double t;
		struct iovec vec;
	} dat;
	size_t len = lx->ivp.pint + 1;
	
	dat.t = lx->t;
	dat.vec.iov_base = lx->y;
	dat.vec.iov_len  = len * lx->ivp.neqs * sizeof(double);
	
	pr.name = 0;
	pr.desc = MPT_tr("LIMEX solver state");
	pr.val.fmt = fmt;
	pr.val.ptr = &dat;
	out(usr, &pr);
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "t";
	pr.desc = "value of independent variable";
	pr.val.fmt = "d";
	pr.val.ptr = &lx->t;
	out(usr, &pr);
	++line;
	}
	
	if (show & (MPT_SOLVER_ENUM(Status) | MPT_SOLVER_ENUM(Report))) {
	pr.name = "n";
	pr.desc = "integration steps";
	pr.val.fmt = "i";
	pr.val.ptr = &lx->iopt[27];
	out(usr, &pr);
	++line;
	}
	
	if (show & MPT_SOLVER_ENUM(Status)) {
	pr.name = "h";
	pr.desc = "current step size";
	pr.val.fmt = "d";
	pr.val.ptr = &lx->h;
	out(usr, &pr);
	++line;
	}
	
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	pr.name = "feval";
	pr.desc = MPT_tr("function evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &lx->iopt[23];
	out(usr, &pr);
	
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.val.fmt = "i";
	pr.val.ptr = &lx->iopt[28];
	out(usr, &pr);
	
	pr.name = "jfeval";
	pr.desc = "jacobian f eval.";
	pr.val.fmt = "i";
	pr.val.ptr = &lx->iopt[24];
	out(usr, &pr);
	
	pr.name = "ludec";
	pr.desc = MPT_tr("LU decompositions");
	pr.val.fmt = "i";
	pr.val.ptr = &lx->iopt[25];
	out(usr, &pr);
	
	pr.name = "lubsub";
	pr.desc = MPT_tr("LU back-subst.");
	pr.val.fmt = "i";
	pr.val.ptr = &lx->iopt[26];
	out(usr, &pr);
	
	return line + 6;
}
