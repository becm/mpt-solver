/*!
 * status information for MINPACK instance
 */

#include <errno.h>

#include "minpack.h"

extern int mpt_minpack_report(const MPT_SOLVER_STRUCT(minpack) *mpack, int show, MPT_TYPE(PropertyHandler) out, void *usr)
{
	MPT_STRUCT(property) pr;
	int line = 0;
	
	if (show & MPT_SOLVER_ENUM(Header)) {
	const char *solv = "powell hybrid", *jac = "numeric";
	
	switch (mpack->solv) {
	    case 0: return 0;
	    case MPT_ENUM(MinpackHybrd):
		if (mpack->mu < mpack->nls.nres || mpack->ml < mpack->nls.nres)
			jac = "banded";
		break;
	    case MPT_ENUM(MinpackHybrj): jac = "user"; break;
	    case MPT_ENUM(MinpackLmDif): solv = "levenberg-marquardt"; break;
	    case MPT_ENUM(MinpackLmDer): solv = "levenberg-marquardt"; jac = "user"; break;
	    case MPT_ENUM(MinpackLmStr): solv = "levenberg-marquardt"; jac = "columnwise user"; break;
	    default:
		errno = EINVAL; return -1;
	}
	
	pr.name = "method";
	pr.desc = MPT_tr("minpack solver type");
	pr.fmt  = 0;
	pr.data = solv;
	out(usr, &pr);
	++line;
	
	pr.name = "jacobian";
	pr.desc = MPT_tr("jacobian matrix type");
	pr.fmt  = 0;
	pr.data = jac;
	out(usr, &pr);
	++line;
	}
	if (!(show & MPT_SOLVER_ENUM(Report))) return line;
	
	
	pr.name = "feval";
	pr.desc = MPT_tr("f evaluations");
	pr.fmt  = "i";
	pr.data = &mpack->nfev;
	out(usr, &pr);
	
	if (!mpack->njev) return line + 1;
	pr.name = "jeval";
	pr.desc = MPT_tr("jacobian evaluations");
	pr.fmt  = "i";
	pr.data = &mpack->njev;
	out(usr, &pr);
	return line + 2;
}
