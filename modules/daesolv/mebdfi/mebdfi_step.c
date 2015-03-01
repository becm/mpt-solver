/*!
 * C wrapper to MEBDFI fortran routine
 */

#include <errno.h>

#include "mebdfi.h"

extern int mpt_mebdfi_step(MPT_SOLVER_STRUCT(mebdfi) *data, double *val, double tend)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *atol, *rtol, tout = tend;
	int itol, mf, ierr = 0, neqs, idid, lout, maxder, liw, lrw;
	
	if ( data->state < 0 ) {
		errno = EINVAL; return -1;
	}
	if ( !data->fcn ) {
		errno = EFAULT; return -1;
	}
	neqs = ivp->neqs * (ivp->pint + 1);
	
	/* set tolerance flags and adresses */
	if (!(rtol = data->rtol.base)) {
		if ((atol = data->atol.base)) {
			itol = 3;
		} else {
			itol = 2; atol = &data->atol.d.val;
		}
		rtol = &data->rtol.d.val;
	}
	else if ((atol = data->atol.base)) {
		itol = 5;
	} else {
		itol = 4; atol = &data->atol.d.val;
	}
	if (!data->jac) {
		data->jnum = 1;
	}
	mf = data->jbnd ? (data->jnum ? 24 : 23) : (data->jnum ? 22 : 21);
	
	/* initial call */
	if ((idid = data->state) == 1 && !data->h) {
		data->h = (tend - ivp->last) / 128;
	}
	liw = data->iwork.iov_len / sizeof(int);
	lrw = data->rwork.iov_len / sizeof(double);
	lout = data->lout;
	maxder = data->maxder;
	
	/* fortran routine call */
	mebdfi_(&neqs, &ivp->last, &data->h, val, data->yp.iov_base, &tout, &tend, &mf,
	        &idid, &lout, &lrw, data->rwork.iov_base, &liw, data->iwork.iov_base,
	        data->mbnd, &maxder, &itol, rtol, atol, data->rpar, data->ipar,
	        data->jac, data->fcn, &ierr);
	
	
	data->state = data->type;
	
	return ierr ? ierr : (idid < 0 ? idid : 0);
}
