/*!
 * call MINPACK routine for problem parameter
 */

#include <errno.h>

#include "../minpack.h"
#include "minpack.h"

extern int mpt_minpack_solve(MPT_SOLVER_STRUCT(minpack) *mpack)
{
	MPT_NLS_STRUCT(parameters) *nl = &mpack->nls;
	double *wa1, *wa2, *wa3, *wa4, *qtf, *fjac, *r;
	double *x, *fvec;
	int n, m, lr, nfev = 0, njev = 0, mode, nprint, info;
	
	if (!mpack->fcn.hd || !(x = mpack->val.iov_base)) {
		return MPT_ERROR(BadArgument);
	}
	if ((n = nl->nval) < 1) {
		return MPT_ERROR(BadArgument);
	}
	m = nl->nres ? nl->nres : n;
	
	fvec = x + n;
	
	lr = mpack->work.iov_len / sizeof(double);
	
	qtf = mpack->work.iov_base; lr -= n;
	wa1 = qtf + n; lr -= n;
	wa2 = wa1 + n; lr -= n;
	wa3 = wa2 + n; lr -= n;
	wa4 = wa3 + n; lr -= nl->nres;
	
	n *= m;
	fjac = wa4 + nl->nres; lr -= n;
	
	r = fjac + n;
	
	mode   = mpack->mode;
	nprint = mpack->nprint;
	
	if (mpack->solv == MPT_ENUM(MinpackHybrd)) {
		hybrd_(mpack->fcn.hd, &nl->nval, x, fvec,
		       &mpack->xtol, &mpack->maxfev,
		       &mpack->ml, &mpack->mu, &mpack->epsfcn, mpack->diag.iov_base,
		       &mode, &mpack->factor, &nprint, &info,
		       &nfev, fjac, &nl->nres, r, &lr, qtf, wa1, wa2, wa3, wa4);
		
		switch (mpack->info = info) {
		    case 0: errno = EINVAL; return -1; 
		    case 2: info =  1; break;
		    case 3: info = -2; errno = ERANGE; break;
		    default: info = 0;
		}
	}
	else if (mpack->solv == MPT_ENUM(MinpackHybrj)) {
		hybrj_(mpack->fcn.hj, &nl->nval, x, fvec, fjac, &m,
		       &mpack->xtol, &mpack->maxfev,
		       mpack->diag.iov_base,
		       &mode, &mpack->factor, &nprint, &info,
		       &nfev, &njev, r, &lr, qtf, wa1, wa2, wa3, wa4);
		
		switch (mpack->info = info) {
		    case 0: errno = EINVAL; return -1; 
		    case 2: info =  1; break;
		    case 3: info = -2; errno = ERANGE; break;
		    default: info = 0;
		}
	}
	else if (mpack->solv == MPT_ENUM(MinpackLmDif)) {
		lmdif_(mpack->fcn.dif, &m, &nl->nval, x, fvec,
		       &mpack->ftol, &mpack->xtol, &mpack->gtol,
		       &mpack->maxfev, &mpack->epsfcn, mpack->diag.iov_base, &mode, &mpack->factor, 
		       &nprint, &info, &nfev, fjac, &m,
		       (int *) r, qtf, wa1, wa2, wa3, wa4);
		
		switch (mpack->info = info) {
		    case 0: errno = EINVAL; return -1; 
		    case 5: info =  1; break;
		    case 6: info = -2; errno = ERANGE; break;
		    case 7: info = -3; errno = ERANGE; break;
		    case 8: info = -4; errno = ERANGE; break;
		    default: info = 0;
		}
	}
	else if (mpack->solv == MPT_ENUM(MinpackLmDer)) {
		lmder_(mpack->fcn.der, &m, &nl->nval, x, fvec, fjac, &m,
		       &mpack->ftol, &mpack->xtol, &mpack->gtol, 
		       &mpack->maxfev, mpack->diag.iov_base, &mode, &mpack->factor,
		       &nprint, &info, &nfev, &njev,
		       (int *) r, qtf, wa1, wa2, wa3, wa4);
		
		switch (mpack->info = info) {
		    case 0: errno = EINVAL; return -1; 
		    case 5: info =  1; break;
		    case 6: info = -2; errno = ERANGE; break;
		    case 7: info = -3; errno = ERANGE; break;
		    case 8: info = -4; errno = ERANGE; break;
		    default: info = 0;
		}
	}
	else if (mpack->solv == MPT_ENUM(MinpackLmStr)) {
		lmstr_(mpack->fcn.str, &m, &nl->nval, x, fvec, fjac, &m,
		       &mpack->ftol, &mpack->xtol, &mpack->gtol,
		       &mpack->maxfev, mpack->diag.iov_base, &mode, &mpack->factor,
		       &nprint, &info, &nfev, &njev,
		       (int *) r, qtf, wa1, wa2, wa3, wa4);
		
		switch (mpack->info = info) {
		    case 0: errno = EINVAL; return -1; 
		    case 5: info =  1; break;
		    case 6: info = -2; errno = ERANGE; break;
		    case 7: info = -3; errno = ERANGE; break;
		    case 8: info = -4; errno = ERANGE; break;
		    default: info = 0;
		}
	}
	else {
		errno = EINVAL;
		return -1;
	}
	mpack->nfev += nfev;
	mpack->njev += njev;
	
	return info;
}

