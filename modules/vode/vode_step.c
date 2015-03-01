/*!
 * C wrapper to VODE fortran routine.
 */

#include <errno.h>
#include <string.h>

#include "vode.h"

extern int mpt_vode_step(MPT_SOLVER_STRUCT(vode) *data, double *val, double tend)
{
	MPT_SOLVER_STRUCT(ivppar) *ivp = &data->ivp;
	double *atol, *rtol;
	int neqs, itol, mf, iopt, itask, istate, liw, lrw;
	
	if (data->istate < 0) {
		errno = EINVAL;
		return -1;
	}
	if (!data->fcn) {
		errno = EFAULT;
		return -1;
	}
	neqs = ivp->neqs * (ivp->pint + 1);
	
	/* set tolerance flags and adresses */
	if (!(rtol = data->rtol.base)) {
		if (!(atol = data->atol.base)) {
			itol = 1; atol = &data->atol.d.val;
		}
		else {
			itol = 2;
		}
		rtol = &data->rtol.d.val;
	}
	else if (!(atol = data->atol.base)) {
		itol = 3; atol = &data->atol.d.val;
	}
	else {
		itol = 4;
	}
	iopt = data->iopt ? 1 : 0;
	itask = data->itask;
	istate = data->istate;
	
	/* setup iteration method */
	switch (mf = data->miter) {
		case 1: if (data->jac) break;
		case 2: mf = 2; break;
		case 3: break;
		case 4: if (data->jac) break;
		case 5: mf = 5; break;
		default: mf = 0;
	}
	mf += ( data->meth == 2 ) ? 20 : 10;
	mf *= data->jsv;
	
	liw = data->iwork.iov_len / sizeof(int);
	lrw = data->rwork.iov_len / sizeof(double);
	
	/* fortran routine call */
	dvode_(data->fcn, &neqs, val, &ivp->last, &tend, &itol, rtol, atol,
	       &itask, &istate, &iopt,
	       data->rwork.iov_base, &lrw, data->iwork.iov_base, &liw,
	       data->jac, &mf, data->rpar, data->ipar);
	
	if (istate >= 0) {
		data->istate = istate;
		return 0;
	}
	return istate;
}
